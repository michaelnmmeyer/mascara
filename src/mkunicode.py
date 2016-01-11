#!/usr/bin/env python3

"""
Generates machines for matching Latin letters

We don't cover all possible cases. In particular, we only include basic
combinining diacritics (Combining Diacritical Marks). The goal is to keep the
generated tokenizer small.
"""

import os, sys
from urllib.request import urlopen as garbage

DERIVED_PROPS = "http://www.unicode.org/Public/UNIDATA/DerivedCoreProperties.txt"
SCRIPT_PROPS  = "http://www.unicode.org/Public/8.0.0/ucd/Scripts.txt"
UNI_DATA      = "ftp://ftp.unicode.org/Public/UNIDATA/UnicodeData.txt"

def urlopen(url):
   fp = garbage(url)
   for line in fp:
      yield line.decode()

def from_hex(x):
   return int(x, 16)

def load_unidata():
   map = {}
   for line in urlopen(UNI_DATA):
      code_point, name, _ = line.split(";", 2)
      map[from_hex(code_point)] = name
   return map

def iter_code_points(char_range):
   range_bounds = [from_hex(x) for x in char_range.split("..")]
   if len(range_bounds) == 1:
      range_bounds.append(range_bounds[0])
   assert len(range_bounds) == 2
   for c in range(range_bounds[0], range_bounds[1] + 1):
      yield c

def iter_property_code_points(property, file):
   # Interesting lines are of the form:
   # 00B5          ; Alphabetic # ...
   # 0041..005A    ; Alphabetic # ...
   for line in urlopen(file):
      line = line.strip()
      if not line or line.startswith("#"):
         continue
      line = line.split("#", 1)[0].strip()
      char_range, prop_name = [x.strip() for x in line.split(";", 1)]
      if prop_name != property:
         continue
      yield from iter_code_points(char_range)

def latin_letters():
   alpha = iter_property_code_points("Alphabetic", DERIVED_PROPS)
   latin = iter_property_code_points("Latin",  SCRIPT_PROPS)
   return set(alpha) & set(latin)

def combining_diacritics():
   my_range = "0300..036F"
   return set(iter_code_points(my_range))

head = """\
/* Generated file, don't edit! */
%%%%{
   machine %s;

   %s ="""
tail = """\
 ;
}%%"""
def dump_machine(fp, name, code_points):
   print(head % (name, name), file=fp)
   sep = " "
   for c in code_points:
      bytes = [hex(b) for b in chr(c).encode("UTF-8")]
      print(" %s %s" % (sep, " ".join(bytes)), file=fp)
      sep = "|"
   print(tail, file=fp)

def dump_code_points(name, code_points):
   code_points = sorted(code_points)
   this_dir = os.path.dirname(__file__)
   out_file = os.path.join(this_dir, "gen", name)
   with open(out_file + ".rl", "w") as fp:
      dump_machine(fp, name, code_points)
   uni_data = load_unidata()
   # Also dump a description of the machine for debugging.
   with open(out_file + ".txt", "w") as fp:
      for c in code_points:
         print(uni_data[c], file=fp)

dump_code_points("raw_latin_letter", latin_letters())
dump_code_points("combining_diacritic", combining_diacritics())
