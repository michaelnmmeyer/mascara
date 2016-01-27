#!/usr/bin/env python3

# Makes a ragel-generated machine read backwards the string it is given as
# argument. This is brittle as hell, but there is no other way.

import sys

def reverse_machine(code):
   to_replace = [
      ("if ( ++p != pe )", "if ( --p != pe )"),
      ("te = p+1", "te = p-1"),
      ("{te = p;p--;{", "{te = p;p++;{"),
      ("{{p = ((te))-1;}", "{{p = ((te))+1;}")
   ]
   for before, after in to_replace:
      code = code.replace(before, after)
   return code

path = sys.argv[1]

with open(path, encoding="UTF-8") as fp:
   code = reverse_machine(fp.read())

with open(path, "w", encoding="UTF-8") as fp:
   fp.write(code)
