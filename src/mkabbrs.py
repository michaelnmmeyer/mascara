#!/usr/bin/env python3

"""
Generates a Ragel machine matching a list of abbreviations. Abbreviations should
end with a period. We don't generate case variants (this is probably not a good
idea).
"""

import sys

ABBRS = set()

for line in sys.stdin:
   abbr = line.strip()
   if not abbr.startswith("#"):
      abbr = abbr.rstrip(".")
      ABBRS.add(abbr)

TPL = """\
/* Generated file, don't edit! */
%%{

machine abbr_lexicon;

abbr_lexicon =
DATA
;

}%%"""

print(TPL.replace("DATA", " |\n".join('"%s."' % abbr for abbr in sorted(ABBRS))))
