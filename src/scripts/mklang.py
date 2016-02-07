#!/usr/bin/env python3

import os, sys, unicodedata

TPL = """\
/* Generated file, don't edit! */
%%{

machine LANG_CATEGORY;

LANG_CATEGORY =
DATA
;

}%%
"""

# Keep that in sync with symbol.rl!
HYPHEN = (
   "\N{HYPHEN-MINUS}",
   "\N{HYPHEN}",
   "\N{NON-BREAKING HYPHEN}",
   "\N{SMALL HYPHEN-MINUS}",
)

# Keep that in sync with symbol.rl!
APOSTROPHE = (
   "'",
   "‘",
   "’",
)

def mkregex(word):
   buf = []
   for c in word:
      assert c != '"'
      c = c.lower()
      # Apostrophe.
      if c == "'":
         buf.append('(apostrophe)')
      elif c == "-":
         buf.append("(hyphen)")
      # Ligatures, case-insensitive.
      elif c == "œ":
         buf.append('("Œ"|"œ"|"OE"i)')
      elif c == "æ":
         buf.append('("Æ"|"æ"|"AE"i)')
      # Make all letters match both uppercase and lowercase equivalents.
      elif c.isalpha():
         buf.append('("%s"|"%s")' % (c.upper(), c.lower()))
      else:
         buf.append('("%s")' % c)
   return "(%s)" % "".join(buf)

# For suffixes.
def mkreversed_regex(form):
   def append(buf, *letters):
      assert letters
      reversed_letters = []
      for letter in letters:
         # "é" = 195 169 -> 169.195
         letter = ".".join("%d" % byte for byte in reversed(letter.encode("UTF-8")))
         reversed_letters.append(letter)
      buf.append("(%s)" % "|".join(reversed_letters))

   buf = []
   for c in form:
      c = c.lower()
      # Apostrophe.
      if c == "'":
         append(buf, *APOSTROPHE)
      elif c == "-":
         append(buf, *HYPHEN)
      # Ligatures
      elif c == "œ":
         append(buf, "Œ", "œ", "OE", "Oe", "oE", "oe")
      elif c == "æ":
         append(buf, "Æ", "æ", "AE", "Ae", "aE", "ae")
      # Letters matching uppercase and lowercase.
      elif c.isalpha():
         append(buf, c.upper(), c.lower())
      else:
         append(buf, c)
   return "(%s)" % "".join(reversed(buf))

def mkmachine(tks, lang, category):
   if category == "suffix":
      f = mkreversed_regex
   else:
      f = mkregex
   s = "|\n".join(f(tk) for tk in tks)
   m = TPL
   m = m.replace("DATA", s or "empty")
   m = m.replace("LANG", lang)
   m = m.replace("CATEGORY", category)
   return m

def strip_comment(line):
   end = len(line)
   pos = -1
   while True:
      pos = line.find("#", pos + 1)
      if pos < 0:
         break
      if pos == 0 or (pos > 0 and line[pos - 1] != "\\"):
         end = pos
         break
   return line[:end]

# Quick'n'dirty testing.
assert strip_comment("c\# #foobar") == "c\# "

def load_words(fp):
   words = []
   for word in fp:
      word = strip_comment(word).replace("\#", "#").strip()
      word = unicodedata.normalize("NFC", word)
      if word:
         words.append(word)
   return words

path = sys.argv[1]
name = os.path.splitext(os.path.basename(path))[0]
with open(path, encoding="UTF-8") as fp:
   tks = load_words(fp)

lang, category = name.split("_")
machine = mkmachine(tks, lang, category)
sys.stdout.buffer.write(machine.encode())
