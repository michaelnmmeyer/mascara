#!/usr/bin/env python3

"""
Generates a Ragel machine matching a list of abbreviations. Only abbreviations
that end with a period must be used. We don't generate case variants (this is
probably not a good idea).
"""

import sys

ABBRS = set()

for line in sys.stdin.buffer:
   abbr = line.decode("UTF-8").strip()
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

ret = TPL.replace("DATA", " |\n".join('"%s."' % abbr for abbr in sorted(ABBRS)))
sys.stdout.buffer.write(ret.encode("UTF-8"))
