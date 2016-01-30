#ifndef CMD_H
#define CMD_H

#define CMD_VERSION "0.4"

#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdnoreturn.h>

extern const char *g_progname;

noreturn void die(const char *, ...)
#ifdef __GNUC__
   __attribute__((format(printf, 1, 2)))
#endif
;

void complain(const char *, ...)
#ifdef __GNUC__
   __attribute__((format(printf, 1, 2)))
#endif
;

struct command {
   const char *name;    // NULL serves as a sentinel.
   void (*func)(int argc, char **argv);
};

/* For programs that take commands as first argument.
   A help screen must be provided. The -h, --help option is the only one that is
   taken care of here. The matching command, if any, is run with the original
   argc and argv shifted by 2. It must handle its own arguments parsing.
 */
noreturn void parse_command(struct command *commands, const char *help_screen,
                            int argc, char **argv);

enum option_type {
   OPT_STR,
   OPT_BOOL,
   OPT_FUNC,
   OPT_DOUBLE,
   OPT_SIZE_T,
   NUM_OPTS
};

struct option {
   int letter;             // Optional.
   const char *name;       // Required, NULL serves as a sentinel.
   enum option_type type;
   union {
      const char **s;
      bool *b;          // The pointed-to boolean must be initialized to false.
      void (*f)(void);
      double *d;
      size_t *z;
   };
};

#define OPT_STR(val)    .type = OPT_STR,     .s = &val
#define OPT_BOOL(val)   .type = OPT_BOOL,    .b = &val
#define OPT_FUNC(val)   .type = OPT_FUNC,    .f =  val
#define OPT_DOUBLE(val) .type = OPT_DOUBLE,  .d = &val
#define OPT_SIZE_T(val) .type = OPT_SIZE_T,  .z = &val

/* Options parsing.
   The "options" array must end with a zeroed option. If it is NULL, merely the
   -h, --help option is handled.
   If "help_screen" is given, we assume that this function is called from main()
   and shift the argument vector accordingly. Otherwise, we assume that
   parse_command() was called first and that this function is called from one of
   the command handlers, in which case the first argument is at argv[0].
   The help screen should contain exactly one formatting directive: %s, for the
   program name. Having one or several %% is okay, though.
   If the same option is given multiple times, we only retain the last value.
 */
void parse_options(struct option *options, const char *help_screen,
                   int *argc, char ***argv);

#endif
