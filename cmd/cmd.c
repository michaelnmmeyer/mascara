#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>

#include "cmd.h"

const char *g_progname = "program";

static void inform(const char *msg, va_list ap)
{
   int my_errno = errno;

   fprintf(stderr, "%s: ", g_progname);
   vfprintf(stderr, msg, ap);
   
   size_t len = strlen(msg);
   if (len && msg[len - 1] == ':')
      fprintf(stderr, " %s", my_errno ? strerror(my_errno) : "<unknown error>");
   
   putc('\n', stderr);
}

noreturn void die(const char *msg, ...)
{
   va_list ap;
   va_start(ap, msg);
   inform(msg, ap);
   va_end(ap);

   exit(EXIT_FAILURE);
}

void complain(const char *msg, ...)
{
   va_list ap;
   va_start(ap, msg);
   inform(msg, ap);
   va_end(ap);
}

static void set_progname(const char *name)
{
   const char *slash = strrchr(name, '/');

   if (slash && slash[1])
      g_progname = slash + 1;
   else
      g_progname = name;
}

static const char *g_help_screen;     // Don't care about encapsulation.

noreturn static void display_help(void)
{
   printf(g_help_screen, g_progname);
   exit(EXIT_SUCCESS);
}

static struct option empty_option = {
   '\0', 0, OPT_FUNC(0)
};

static struct option help_option = {
   'h', "help", OPT_FUNC(display_help)
};

static struct option *short_option(struct option *options, int letter)
{
   assert(letter);
   
   while (options->name) {
      if (options->letter == letter)
         return options;
      options++;
   }
   if (letter == help_option.letter)
      return &help_option;
   
   die("unknown option: -%c", letter);
}

static struct option *long_option(struct option *options, const char *name)
{
   while (options->name) {
      if (!strcmp(options->name, name))
         return options;
      options++;
   }
   if (!strcmp(name, help_option.name))
      return &help_option;
   
   die("unknown option: --%s", name);
}

static void handle_option(struct option *opt, const char *arg)
{
   bool legit = true;
   
   switch (opt->type) {
   case OPT_STR:
      *opt->s = arg;
      break;
   case OPT_BOOL:
      *opt->b = true;
      break;
   case OPT_FUNC:
      opt->f();
      break;
   case OPT_DOUBLE: {
      char *end;
      errno = 0;
      *opt->d = strtod(arg, &end);
      if (errno || *end)
         legit = false;
      break;
   }
   case OPT_SIZE_T:
      if (!strcmp(arg, "inf")) {
         *opt->z = SIZE_MAX;
      } else {
         char *end;
         errno = 0;
         long long num = strtoll(arg, &end, 10);
         if (errno || *end || num < 0 || (uintmax_t)num > SIZE_MAX)
            legit = false;
         else
            *opt->z = num;
      }
      break;
   default:       // NUM_OPTS handled in switch.
      break;
   }
   
   if (!legit)
      die("invalid argument '%s' for option --%s", arg, opt->name);
}

static const bool needs_arg[NUM_OPTS] = {
   [OPT_STR] = true,
   [OPT_DOUBLE] = true,
   [OPT_SIZE_T] = true,
};

#ifndef NDEBUG
static void check_help(const char *help)
{
   assert(help);
   bool legit = false;
   
   for (;;) {
      help = strchr(help, '%');
      if (!help)
         break;
      switch (*++help) {
      case 's':
         if (legit) {
            legit = false;
            goto fini;
         }
         legit = true;
         break;
      case '%':
         help++;
         break;
      default:
         legit = false;
         goto fini;
      }
   }
   
fini:
   if (!legit)
      die("invalid help screen");
}
#endif

static void set_help(const char *help)
{
#ifndef NDEBUG
   check_help(help);
#endif
   g_help_screen = help;
}

noreturn void parse_command(struct command *commands, const char *help,
                            int argc, char **argv)
{
   set_progname(*argv);
   set_help(help);
   
   if (argc == 1)
      display_help();
   
   const char *arg = argv[1];
   for (struct command *cmd = commands; cmd->name; cmd++) {
      if (!strcmp(cmd->name, arg)) {
         cmd->func(argc - 2, argv + 2);
         exit(EXIT_SUCCESS);
      }
   }
   if (*arg == '-') {
      arg++;
      if (*arg == '-') {
         arg++;
         if (!strcmp(arg, help_option.name))
            display_help();
      } else if (*arg == help_option.letter && !arg[1]) {
         display_help();
      }
   }
   die("unknown command: '%s'", argv[1]);
}

void parse_options(struct option *options, const char *help,
                   int *argcp, char ***argvp)
{
   if (!options)
      options = &empty_option;

   int argc = *argcp;
   char **argv = *argvp;
   
   int i;
   if (help) {
      assert(argc > 0);
      set_progname(*argv);
      set_help(help);
      i = 1;
   } else {
      assert(g_help_screen);
      i = 0;
   }

   for ( ; i < argc && argv[i][0] == '-'; i++) {
      char *opt = argv[i];
      if (!opt[1]) {
         break;
      } else if (opt[1] != '-') {
         opt++;
         struct option *option = short_option(options, *opt);
         while (!needs_arg[option->type] && opt[1]) {
            handle_option(option, help);
            option = short_option(options, *++opt);
         }
         char *arg = opt + 1;
         if (needs_arg[option->type]) {
            if (!*arg) {
               arg = argv[++i];
               if (!arg || *arg == '-')
                  die("option -%c requires an argument", *opt);
            }
         } else if (*arg) {
            die("option -%c doesn't accept an argument, have '%s'", *opt, arg);
         }
         handle_option(option, arg);
      } else if (!opt[2]) {
         i++;
         break;
      } else {
         opt += 2;
         char *arg = strchr(opt, '=');
         if (arg)
            *arg++ = '\0';
         struct option *option = long_option(options, opt);
         if (needs_arg[option->type]) {
            if (!arg) {
               arg = argv[++i];
               if (!arg || *arg == '-')
                  die("option --%s requires an argument", opt);
            }
         } else if (arg) {
            die("option --%s doesn't accept an argument, have '%s'", opt, arg);
         }
         handle_option(option, arg);
      }
   }
   
   *argcp -= i;
   *argvp += i;
}
