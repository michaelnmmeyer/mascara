#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../mascara.h"
#include "../src/lib/kabak.h"
#include "cmd.h"
#include "print_str.ic"

static void print_format(const char *fmt, const struct mr_token *tk)
{
   switch (*fmt) {
   case 't':
      fputs(mr_type_name(tk->type), stdout);
      break;
   case 's':
      print_str((const unsigned char *)tk->str, tk->len);
      break;
   case 'l':
      printf("%zu", tk->len);
      break;
   case 'o':
      printf("%zu", tk->offset);
      break;
   case '%':            /* '%' + '%' */
   case '\0':           /* '%' at the end of the format string. */
      putchar('%');
      break;
   default:
      putchar('%');     /* '%' + some other character. */
      putchar(*fmt);
      break;
   }
}

static void print_escaped(const char *fmt)
{
   switch (*fmt) {
   #define _(c1, c2) case c1: putchar(c2); break;
   _('n', '\n')
   _('t', '\t')
   _('\\', '\\' )
   _('\0', '\\')         /* '\' at the end of the format string. */
   #undef _
   default:
      putchar('\\');     /* '\' + some other character. */
      putchar(*fmt);
      break;
   }
}

static void print_token(const char *fmt, const struct mr_token *tk)
{
   for ( ; *fmt; fmt++) {
      switch (*fmt) {
      case '%':
         print_format(++fmt, tk);
         break;
      case '\\':
         print_escaped(++fmt);
         break;
      default:
         putchar(*fmt);
         break;
      }
   }
}

static void print_eos(const char *s)
{
   for ( ; *s; s++) {
      switch (*s) {
      case '\\':
         print_escaped(++s);
         break;
      default:
         putchar(*s);
         break;
      }
   }
}

#define MAX_FILE_SIZE (50 * 1024 * 1024)

static void *read_file(const char *path, size_t *sizep)
{
   FILE *fp = stdin;
   if (path) {
      fp = fopen(path, "r");
      if (!fp)
         die("cannot open '%s':", path);
   }
   char *str = malloc(MAX_FILE_SIZE); // FIXME later
   size_t len = fread(str, 1, MAX_FILE_SIZE, fp);

   if (ferror(fp))
      die("IO error:");
   if (len == MAX_FILE_SIZE && !feof(fp))
      die("input file too large (limit is %d)", MAX_FILE_SIZE);

   if (fp != stdin)
      fclose(fp);

   struct kabak nrm = KB_INIT;
   kb_transform(&nrm, str, len, 0);
   free(str);

   return kb_detach(&nrm, sizep);
}

noreturn static void version(void)
{
   const char *msg =
   "Mascara version "MR_VERSION"\n"
   "Copyright (c) 2016 Michaël Meyer"
   ;
   puts(msg);
   exit(EXIT_SUCCESS);
}

static void tokenize_with_eos(struct mascara *mr,
                              const char *fmt, const char *eos)
{
   struct mr_token *tks;
   size_t len;

   while ((len = mr_next(mr, &tks))) {
      for (size_t i = 0; i < len; i++)
         print_token(fmt, &tks[i]);
      print_eos(eos);
   }
}

static void tokenize_without_eos(struct mascara *mr, const char *fmt)
{
   struct mr_token *tk;

   while (mr_next(mr, &tk))
      print_token(fmt, tk);
}

static void tokenize(const char *path, const char *lang,
                     const char *fmt, const char *eos)
{
   enum mr_mode mode = *eos ? MR_SENTENCE : MR_TOKEN;
   struct mascara *mr;
   int ret = mr_alloc(&mr, lang, mode);
   if (ret)
      die("cannot create tokenizer: %s", mr_strerror(ret));

   size_t len;
   char *str = read_file(path, &len);

   mr_set_text(mr, str, len);
   if (mode == MR_TOKEN)
      tokenize_without_eos(mr, fmt);
   else
      tokenize_with_eos(mr, fmt, eos);

   free(str);
   mr_dealloc(mr);
}

static void display_langs(void)
{
   const char *const *langs = mr_langs();
   while (*langs)
      puts(*langs++);
}

int main(int argc, char **argv)
{
   const char *fmt = "%s\n";
   const char *lang = "en";
   const char *eos = "\n";
   bool list = false;

   struct option opts[] = {
      {'f', "format", OPT_STR(fmt)},
      {'l', "lang", OPT_STR(lang)},
      {'L', "list", OPT_BOOL(list)},
      {'e', "eos", OPT_STR(eos)},
      {'\0', "version", OPT_FUNC(version)},
      {0},
   };
   const char help[] =
      #include "mascara.ih"
   ;

   parse_options(opts, help, &argc, &argv);
   if (argc > 1)
      die("excess arguments");
   
   const char *home = getenv("MR_HOME");
   mr_home = home ? home : MR_HOME;

   if (list)
      display_langs();
   else
      tokenize(*argv, lang, fmt, eos);
}
