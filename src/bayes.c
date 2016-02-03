#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>
#include "bayes.h"
#include "api.h"
#include "mem.h"

struct feature {
   double probs[2];
   struct feature *next;
   char value[];
};

struct bayes {
   double priors[2];
   double *unknown_probs;
   size_t table_mask;
   const struct bayes_config *cfg;  /* For debugging. */
   struct feature *table[];
};

#define MAX_STRING_LEN 512
#define MAX_FEATURES (0xff - 1)
#define MAX_VALUES 10000

/* <feature_name> <eos_unknown_prob> <not_eos_unknown_prob> */
local int read_name(FILE *fp, const char *name, double unk_probs[static 2])
{
   char buf[MAX_STRING_LEN + 1];
   size_t len;

   if (fscanf(fp, "%zu:", &len) != 1 || len == 0 || len >= sizeof buf)
      return MR_EMODEL;
   if (fread(buf, 1, len, fp) != len)
      return MR_EMODEL;
   buf[len] = '\0';
   if (strcmp(buf, name))
      return MR_EMODEL;
   if (fscanf(fp, "%la %la\n", &unk_probs[EOS], &unk_probs[NOT_EOS]) != 2)
      return MR_EMODEL;
   return MR_OK;
}

/* <feature_no> <feature_value> <eos_prob> <not_eos_prob> */
local int read_feature(struct feature **ff, FILE *fp, unsigned feat_nr)
{
   unsigned feat_no;
   size_t len;
   struct feature *f = NULL;

   if (fscanf(fp, "%u %zu:", &feat_no, &len) != 2)
      goto fail;
   if (feat_no >= feat_nr || len > MAX_FEATURE_LEN)
      goto fail;

   f = mr_malloc(sizeof *f + 1 + len + 1);
   f->next = NULL;
   *f->value = feat_no + 1;
   if (fread(f->value + 1, 1, len, fp) != len)
      goto fail;
   f->value[1 + len] = '\0';
   if (strlen(f->value + 1) != len)
      goto fail;
   if (fscanf(fp, "%la %la\n", &f->probs[EOS], &f->probs[NOT_EOS]) != 2)
      goto fail;

   *ff = f;
   return MR_OK;

fail:
   free(f);
   return MR_EMODEL;
}

local int match_signature(FILE *fp, const char *str)
{
   char buf[MAX_STRING_LEN + 1 + 1];   /* signature, '\n', '\0' */
   size_t len;

   if (!fgets(buf, sizeof buf, fp))
      return MR_EMAGIC;
   len = strlen(buf);
   if (len == 0 || buf[len - 1] != '\n')
      return MR_EMAGIC;
   buf[--len] = '\0';
   return strcmp(buf, str) ? MR_EMAGIC : MR_OK;
}

local size_t pad(size_t n, size_t align)
{
   return n + ((n + align - 1) & ~(align - 1));
}
#define pad(n, type) pad(n, alignof(type))

/* http://graphics.stanford.edu/~seander/bithacks.html */
local uint32_t roundup_pow2(uint32_t num)
{
   num--;
   num |= num >> 1;
   num |= num >> 2;
   num |= num >> 4;
   num |= num >> 8;
   num |= num >> 16;
   num++;
   return num ? num : 1;
}

local struct feature **table_chain(const struct bayes *mdl, const void *val)
{
   uint32_t h = 1315423911;
   const uint8_t *v = val;

   for (size_t i = 0; v[i]; i++)
      h ^= (h << 5) + v[i] + (h >> 2);

   struct feature *const *f = &mdl->table[h & mdl->table_mask];
   while (*f && strcmp((*f)->value, val))
      f = &(*f)->next;

   return (void *)f;
}

local void bayes_clear(struct bayes *mdl)
{
   for (size_t i = 0; i < mdl->table_mask + 1; i++) {
      struct feature *f = mdl->table[i];
      while (f) {
         struct feature *n = f->next;
         free(f);
         f = n;
      }
   }
}

local unsigned array_len(const char *const *xs)
{
   unsigned i;
   for (i = 0; xs[i]; i++)
      ;
   return i;
}

local int load_fp(struct bayes **mdlp, FILE *fp, const struct bayes_config *cfg)
{
   if (match_signature(fp, "mr_bayes 1"))
      return MR_EMAGIC;
   
   char sig[MAX_STRING_LEN + 1];
   int len = snprintf(sig, sizeof sig, "%s %s", cfg->name, cfg->version);
   if (len < 0 || (size_t)len >= sizeof sig || match_signature(fp, sig))
      return MR_EMAGIC;

   unsigned feat_nr;
   size_t value_nr;
   if (fscanf(fp, "features %u values %zu\n", &feat_nr, &value_nr) != 2)
      return MR_EMODEL;
   if (feat_nr == 0 || feat_nr > MAX_FEATURES || feat_nr != array_len(cfg->features))
      return MR_EMODEL;
   if (value_nr == 0 || value_nr > MAX_VALUES)
      return MR_EMODEL;

   double priors[2];
   if (fscanf(fp, "EOS %la !EOS %la\n", &priors[EOS], &priors[NOT_EOS]) != 2)
      return MR_EMODEL;

   size_t tbl_size = roundup_pow2(value_nr * 0.7);
   size_t unk_off = pad(sizeof(struct bayes) + tbl_size * sizeof(struct feature *), double);
   size_t total = unk_off + sizeof(double[feat_nr][value_nr]);

   struct bayes *mdl = mr_calloc(1, total);

   int ret = MR_OK;
   mdl->priors[0] = priors[0];
   mdl->priors[1] = priors[1];
   mdl->unknown_probs = (void *)((char *)mdl + unk_off);
   mdl->table_mask = tbl_size - 1;
   mdl->cfg = cfg;

   for (unsigned i = 0; i < feat_nr; i++) {
      const char *name = cfg->features[i];
      ret = read_name(fp, name, mdl->unknown_probs + i * 2);
      if (ret)
         goto fail;
   }
   for (size_t i = 0; i < value_nr; i++) {
      struct feature *f;
      ret = read_feature(&f, fp, feat_nr);
      if (ret) {
         goto fail;
      }
      struct feature **ff = table_chain(mdl, f->value);
      if (*ff) {
         free(f);
         ret = MR_EMODEL;
         goto fail;
      }
      *ff = f;
   }
   if (!feof(fp)) {
      ret = MR_EMODEL;
      goto fail;
   }
   *mdlp = mdl;
   return MR_OK;

fail:
   bayes_dealloc(mdl);
   return ret;
}

local int bayes_load(struct bayes **mdl, const char *path,
                     const struct bayes_config *cfg)
{
   FILE *fp = fopen(path, "r");
   if (!fp)
      return MR_EOPEN;

   int ret = load_fp(mdl, fp, cfg);
   int err = ferror(fp);
   fclose(fp);

   return err ? MR_EIO : ret;
}

local void bayes_dealloc(struct bayes *mdl)
{
   bayes_clear(mdl);
   free(mdl);
}

local const double *bayes_probs(const struct bayes *mdl, const uint8_t *val)
{
   assert(*val > 0);
   const struct feature *f = *table_chain(mdl, val);
   return f ? f->probs : &mdl->unknown_probs[(*val - 1) * 2];
}

local const bool bayes_debug = false;

local void bayes_init(const struct bayes *mdl, double v[static 2])
{
   v[0] = mdl->priors[0];
   v[1] = mdl->priors[1];

   if (bayes_debug) {
      fprintf(stderr, "%s INIT %.3lf %.3lf\n", mdl->cfg->name, v[0], v[1]);
   }
}

local void bayes_feed(const struct bayes *mdl, double v[static 2],
                      const void *val)
{
   const double *restrict x = bayes_probs(mdl, val);
   v[0] += x[0];
   v[1] += x[1];

   if (bayes_debug) {
      const char *sig = mdl->cfg->name;
      const char *nam = mdl->cfg->features[*(uint8_t *)val - 1];
      const char *ft = (const char *)val + 1;
      fprintf(stderr, "%s FEED %s %s %.3lf %.3lf\n", sig, nam, ft, x[0], x[1]);
   }
}
