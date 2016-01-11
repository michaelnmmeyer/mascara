#!/usr/bin/env python3

LANGS = ["en", "fr"]

import os

this_dir = os.path.dirname(__file__)

def read_file(rel_path):
   with open(os.path.join(this_dir, rel_path)) as fp:
      return fp.read()

def write_file(rel_path, data):
   with open(os.path.join(this_dir, rel_path), "w") as fp:
      print("/* Generated file, don't edit! */", file=fp)
      fp.write(data)

tokenize_tpl = read_file("tokenize.tpl")
word_tpl = read_file("word.tpl")
suffix_tpl = read_file("suffix.tpl")

for lang in LANGS:
   tokenize = tokenize_tpl.replace("$LANG", lang)
   if lang != "en":
      tokenize = tokenize.replace("en_assimilation", "void")
   word = word_tpl.replace("$LANG", lang)
   suffix = suffix_tpl.replace("$LANG", lang)
   write_file("gen/tokenize_%s.rl" % lang, tokenize)
   write_file("gen/word_%s.rl" % lang, word)
   write_file("gen/suffix_%s.rl" % lang, suffix)
