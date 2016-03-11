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

noreturn static void version(void)
{
   const char *msg =
   "Mascara version "MR_VERSION"\n"
   "Copyright (c) 2016 Michaël Meyer"
   ;
   puts(msg);
   exit(EXIT_SUCCESS);
}

static const unsigned norm_opts = KB_NFC | KB_STRIP_IGNORABLE;

static void tokenize_with_eos(struct kb_file *fp, struct mascara *mr,
                              const char *fmt, const char *eos)
{
   struct kabak para = KB_INIT;

   while (kb_get_para(fp, &para, norm_opts) != KB_FINI) {
      mr_set_text(mr, para.str, para.len);
      struct mr_token *tks;
      size_t len;
      while ((len = mr_next(mr, &tks))) {
         for (size_t i = 0; i < len; i++)
            print_token(fmt, &tks[i]);
         print_eos(eos);
      }
   }
   kb_fini(&para);
}

static void tokenize_without_eos(struct kb_file *fp, struct mascara *mr,
                                 const char *fmt)
{
   struct kabak para = KB_INIT;

   while (kb_get_para(fp, &para, norm_opts) != KB_FINI) {
      mr_set_text(mr, para.str, para.len);
      struct mr_token *tk;
      while (mr_next(mr, &tk))
         print_token(fmt, tk);
   }
   kb_fini(&para);
}

static void tokenize(enum mr_mode mode, const char *path, const char *lang,
                     const char *fmt, const char *eos)
{
   struct mascara *mr;
   int ret = mr_alloc(&mr, lang, mode);
   if (ret)
      die("cannot create tokenizer: %s", mr_strerror(ret));

   struct kb_file fp;
   if (kb_open(&fp, path))
      die("cannot open input file:");

   if (mode == MR_TOKEN || mode == MR_GRAPHEME)
      tokenize_without_eos(&fp, mr, fmt);
   else
      tokenize_with_eos(&fp, mr, fmt, eos);

   ret = kb_close(&fp);
   if (ret)
      die("could not process input file: %s", kb_strerror(ret));
   mr_dealloc(mr);
}

static void display_langs(void)
{
   const char *const *langs = mr_langs();
   while (*langs)
      puts(*langs++);
}

noreturn static void die_msg(const char *msg)
{
   die("%s", msg);
}

int main(int argc, char **argv)
{
   const char *fmt = "%s\n";
   const char *lang = "en";
   const char *eos = "\n";
   const char *mode = "sentence";
   bool list = false;

   struct option opts[] = {
      {'f', "format", OPT_STR(fmt)},
      {'l', "lang", OPT_STR(lang)},
      {'L', "list", OPT_BOOL(list)},
      {'e', "eos", OPT_STR(eos)},
      {'m', "mode", OPT_STR(mode)},
      {'\0', "version", OPT_FUNC(version)},
      {0},
   };
   const char help[] =
      #include "mascara.ih"
   ;

   parse_options(opts, help, &argc, &argv);
   if (argc > 1)
      die("excess arguments");
   
   enum mr_mode m;
   if (!strcmp(mode, "grapheme"))
      m = MR_GRAPHEME;
   else if (!strcmp(mode, "token"))
      m = MR_TOKEN;
   else if (!strcmp(mode, "sentence"))
      m = MR_SENTENCE;
   else
      die("invalid tokenization mode: '%s'", mode);
   
   const char *home = getenv("MR_HOME");
   mr_home = home ? home : MR_HOME;

   mr_on_error(die_msg);
   if (list)
      display_langs();
   else
      tokenize(m, *argv, lang, fmt, eos);
}
