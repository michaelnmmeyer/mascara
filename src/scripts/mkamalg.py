#!/usr/bin/env python3

"""
Creates an amalgamation of several C source files and prints the result on the
standard output. Only includes of the form:

   #include "foobar.h"

are taken into account. System-wide includes and includes containing a macro:

   #include <foobar.h>
   #include FOOBAR_H

are ignored.

As concerns files inclusion, we can either a) include a source file each time
there is a directive asking for that, or b) include it only the first time. To
do selective inclusions, we'd need to evaluate macros like a real preprocessor,
which is too much trouble.

The solution a) is the safest one, because it can cope with the ugly macro magic
that involves multiple inclusions of the same source file with different defined
macros, etc. But b) produces smaller amalgamated files.
"""

import sys, os, re

# Whether to emit #line preprocessor directives.
EMIT_LINENO = True

# Whether to include a source file only one time.
INCLUDE_ONCE = True

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
         if not INCLUDE_ONCE or new_file not in FILES:
            concat_headers(new_file)
            lineno_emitted = False
      else:
         print(line)

for file in sys.argv[1:]:
   concat_headers(file)
