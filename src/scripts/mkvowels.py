#!/usr/bin/env python3

"""
Generates a C function for matching (some) Latin vowels.
"""

import sys
from subprocess import Popen, PIPE, STDOUT

VOWELS = "aeiouyáéíóúàèìòùäëïöüâêîôûāēīōū"
VOWELS += VOWELS.upper()

LOW = min(ord(c) for c in VOWELS)
HIGH = max(ord(c) for c in VOWELS)

def set_char(bitset, c):
   bkt = c >> 5
   bitset.extend((bkt + 1 - len(bitset)) * [0])
   bitset[bkt] |= (1 << (c & 31))

bitset = []
for c in VOWELS:
   set_char(bitset, ord(c) - LOW)

def print(s=""):
   sys.stdout.buffer.write(s.encode("UTF-8") + b"\n")

print("""\
#include <uchar.h>
#include <stdbool.h>
#include <stdint.h>
""")
print("#define MR_VOWEL_MIN %d" % LOW)
print("#define MR_VOWEL_MAX %d" % HIGH)
print('#define MR_VOWELS U"%s"' % "".join(VOWELS))
print()
print("""\
static bool mr_is_vowel(char32_t c)
{
   static const uint32_t tbl[] = {""")
for num in bitset:
   print("      %d," % num)
print("""\
   };
   
   if (c >= MR_VOWEL_MIN && c <= MR_VOWEL_MAX) {
      c -= MR_VOWEL_MIN;
      return tbl[c >> 5] & (1 << (c & 31));
   }
   return false;
}""")
