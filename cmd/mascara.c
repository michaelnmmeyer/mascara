#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../mascara.h"
#include "cmd.h"

static void print_format(const char *fmt, const struct mr_token *tk)
{
   switch (*fmt) {
   case 't':
      fputs(mr_token_type_name(tk->type), stdout);
      break;
   case 's':
      fwrite(tk->str, 1, tk->len, stdout);
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

static char *read_file(FILE *fp, size_t *sizep)
{
   char *data = malloc(MAX_FILE_SIZE); // FIXME later
   size_t size = fread(data, 1, MAX_FILE_SIZE, fp);

   if (ferror(fp))
      die("IO error:");
   if (size == MAX_FILE_SIZE && !feof(fp))
      die("input file too large (limit is %d)", MAX_FILE_SIZE);

   *sizep = size;
   return data;
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
   struct mr_sentence st = MR_SENTENCE_INIT;

   while (mr_next_sentence(mr, &st)) {
      for (size_t i = 0; i < st.len; i++)
         print_token(fmt, &st.tokens[i]);
      print_eos(eos);
   }
   mr_sentence_fini(&st);
}

static void tokenize_without_eos(struct mascara *mr, const char *fmt)
{
   struct mr_token tk;
   
   while (mr_next_token(mr, &tk))
      print_token(fmt, &tk);
}

int main(int argc, char **argv)
{
   const char *fmt = "%s\n";
   const char *lang = "en";
   const char *eos = "\n";
   struct option opts[] = {
      {'f', "format", OPT_STR(fmt)},
      {'l', "lang", OPT_STR(lang)},
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

   FILE *fp = stdin;
   if (argc == 1) {
      fp = fopen(*argv, "r");
      if (!fp)
         die("cannot open '%s':", *argv);
   }
   size_t len;
   char *str = read_file(fp, &len);
   if (fp != stdin)
      fclose(fp);

   struct mascara *mr = mr_alloc(lang);
   if (!mr)
      die("unknown tokenizer: '%s'", lang);

   mr_set_text(mr, str, len);
   /* Only bother to tokenize sentences if we are asked to. */
   if (*eos)
      tokenize_with_eos(mr, fmt, eos);
   else
      tokenize_without_eos(mr, fmt);
   
   free(str);
   mr_dealloc(mr);
}
