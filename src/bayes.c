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

struct mr_bayes {
   double priors[2];
   double *unknown_probs;
   size_t table_mask;
   const struct mr_bayes_config *cfg;  /* For debugging. */
   struct feature *table[];
};

#define MAX_STRING_LEN 512
#define MAX_FEATURES (0xff - 1)
#define MAX_VALUES 10000

/* <feature_name> <eos_unknown_prob> <not_eos_unknown_prob> */
static int read_name(FILE *fp, const char *name, double unk_probs[static 2])
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
   if (fscanf(fp, "%la %la\n", &unk_probs[MR_EOS], &unk_probs[MR_NOT_EOS]) != 2)
      return MR_EMODEL;
   return MR_OK;
}

/* <feature_no> <feature_value> <eos_prob> <not_eos_prob> */
static int read_feature(struct feature **ff, FILE *fp, unsigned feat_nr)
{
   unsigned feat_no;
   size_t len;
   if (fscanf(fp, "%u %zu:", &feat_no, &len) != 2)
      return MR_EMODEL;
   if (feat_no > feat_nr || len == 0 || len > MAX_STRING_LEN)
      return MR_EMODEL;

   struct feature *f = mr_malloc(sizeof *f + 1 + len + 1);
   f->next = NULL;
   *f->value = feat_no + 1;
   if (fread(f->value + 1, 1, len, fp) != len)
      goto fail;
   f->value[len + 1] = '\0';
   if (strlen(f->value + 1) != len)
      goto fail;
   if (fscanf(fp, "%la %la\n", &f->probs[MR_EOS], &f->probs[MR_NOT_EOS]) != 2)
      goto fail;

   *ff = f;
   return MR_OK;

fail:
   free(f);
   return MR_EMODEL;
}

static int match_signature(FILE *fp, const char *str)
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

static size_t pad(size_t n, size_t align)
{
   return n + ((n + align - 1) & ~(align - 1));
}
#define pad(n, type) pad(n, alignof(type))

/* http://graphics.stanford.edu/~seander/bithacks.html */
static uint32_t roundup_pow2(uint32_t num)
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

static struct feature **table_chain(const struct mr_bayes *mdl, const void *val)
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

static void mr_bayes_clear(struct mr_bayes *mdl)
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

static unsigned mr_array_len(const char *const *xs)
{
   unsigned i;
   for (i = 0; xs[i]; i++)
      ;
   return i;
}

static int load_fp(struct mr_bayes **mdlp, FILE *fp, const struct mr_bayes_config *cfg)
{
   if (match_signature(fp, "mr_bayes 1") || match_signature(fp, cfg->signature))
      return MR_EMAGIC;

   unsigned feat_nr;
   size_t value_nr;
   if (fscanf(fp, "features %u values %zu\n", &feat_nr, &value_nr) != 2)
      return MR_EMODEL;
   if (feat_nr == 0 || feat_nr > MAX_FEATURES || feat_nr != mr_array_len(cfg->features))
      return MR_EMODEL;
   if (value_nr == 0 || value_nr > MAX_VALUES)
      return MR_EMODEL;

   double priors[2];
   if (fscanf(fp, "EOS %la !EOS %la\n", &priors[MR_EOS], &priors[MR_NOT_EOS]) != 2)
      return MR_EMODEL;

   size_t tbl_size = roundup_pow2(value_nr * 0.7);
   size_t unk_off = pad(sizeof(struct mr_bayes) + tbl_size * sizeof(struct feature *), double);
   size_t total = unk_off + sizeof(double[feat_nr][value_nr]);

   struct mr_bayes *mdl = mr_calloc(1, total);

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
   mr_bayes_dealloc(mdl);
   return ret;
}

int mr_bayes_load(struct mr_bayes **mdl, const char *path,
                  const struct mr_bayes_config *cfg)
{
   FILE *fp = fopen(path, "r");
   if (!fp)
      return MR_EOPEN;

   *mdl = NULL;
   int ret = load_fp(mdl, fp, cfg);
   int err = ferror(fp);
   fclose(fp);

   return err ? MR_EIO : ret;
}

void mr_bayes_dealloc(struct mr_bayes *mdl)
{
   mr_bayes_clear(mdl);
   free(mdl);
}

static const double *mr_bayes_probs(const struct mr_bayes *mdl, const uint8_t *val)
{
   const struct feature *f = *table_chain(mdl, val);
   return f ? f->probs : &mdl->unknown_probs[(*val - 1) * 2];
}

static const bool mr_bayes_debug = false;

void mr_bayes_init(const struct mr_bayes *mdl, double v[static 2])
{
   v[0] = mdl->priors[0];
   v[1] = mdl->priors[1];

   if (mr_bayes_debug) {
      fprintf(stderr, "%s: INIT %.3lf %.3lf\n", mdl->cfg->signature, v[0], v[1]);
   }
}

void mr_bayes_feed(const struct mr_bayes *mdl, double v[static 2], const void *val)
{
   const double *restrict x = mr_bayes_probs(mdl, val);
   v[0] += x[0];
   v[1] += x[1];

   if (mr_bayes_debug) {
      const char *sig = mdl->cfg->signature;
      const char *nam = mdl->cfg->features[*(uint8_t *)val - 1];
      const char *ft = (const char *)val + 1;
      fprintf(stderr, "%s: FEED %s %s %.3lf %.3lf\n", sig, nam, ft, x[0], x[1]);
   }
}
