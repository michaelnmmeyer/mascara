#!/usr/bin/env python3

"""
Creates an amalgamation of several C source files and prints the result on the
standard output. Only includes of the form:

   #include "foobar.h"

are taken into account. System-wide includes and includes containing a macro:

   #include <foobar.h>
   #include FOOBAR_H

are ignored.

We don't prevent multiple inclusions because this is too hard. To do that
correctly, we'd need to evaluate macros like:

   #if FOOBAR == 64 % 51 + 32
      #include "foobar.h"
   #endif

The preprocessor can easily discard unused text anyway.
"""

import sys, os, re

EMIT_LINENO = True
INCLUDE_DUPLICATES = False

FILES = {}

def read_file(path):
   code = FILES.get(path)
   if not code:
      with open(path) as fp:
         code = fp.read()
      FILES[path] = code
   return code

def concat_headers(file):
   dirname = os.path.dirname(file)
   basename = os.path.basename(file)
   lines = read_file(file).splitlines()
   lineno_emitted = False
   for lineno, line in enumerate(lines, 1):
      if EMIT_LINENO and not lineno_emitted:
         print('#line %d "%s"' % (lineno, basename))
         lineno_emitted = True
      match = re.match(r'^\s*#\s*include\s+"(.+?)"', line)
      if match:
         new_file = os.path.join(dirname, match.group(1))
         if INCLUDE_DUPLICATES or new_file not in FILES:
            concat_headers(new_file)
            lineno_emitted = False
      else:
         print(line)

for arg in sys.argv[1:]:
   if arg == "-no-lineno":
      EMIT_LINENO = False
   elif arg == "-no-duplicates":
      INCLUDE_DUPLICATES = True
   else:
      continue
   sys.argv.remove(arg)

for file in sys.argv[1:]:
   concat_headers(file)
