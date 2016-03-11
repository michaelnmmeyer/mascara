#!/usr/bin/env python3

import re
from collections import defaultdict
from urllib.request import urlopen

URL = "http://www.unicode.org/Public/UCD/latest/ucd/auxiliary/GraphemeBreakProperty.txt"

def iter_codes():
   with urlopen(URL) as fp:
      for line in fp:
         line = line.decode().strip()
         if not line or line.startswith("#"):
            continue
         span, category = re.match("(\S+)\s*;\s*(\S+)", line).groups()
         span = [int(c, 16) for c in span.split("..")]
         if len(span) == 1:
            start, end = span[0], span[0]
         else:
            start, end = span
         for c in range(start, end + 1):
            yield c, category

def is_surrogate(c):
   return 0xD800 <= c <= 0xDFFF

categories = defaultdict(list)
for code, category in iter_codes():
   if is_surrogate(code):
      continue
   code_hex = ["0x%02X" % b for b in chr(code).encode("UTF-8")]
   categories[category].append(" ".join(code_hex))

print(""""%%{

machine grapheme_break_property;
""")

for name, code_points in sorted(categories.items()):
   print("%s = " % name)
   sep = " "
   for seq in code_points:
      print("%s %s" % (sep, seq))
      sep = "|"
   print(";")
   print()

print("""\
# Currently empty, see http://unicode.org/reports/tr29/#Prepend.
Prepend = empty;

}%%""")
