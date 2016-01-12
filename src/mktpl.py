#!/usr/bin/env python3

import os, sys

def full_path(rel_path):
   this_dir = os.path.dirname(__file__)
   return os.path.join(this_dir, rel_path)

name = sys.argv[1]
lang, category = name.split("_", 1)

with open(full_path(category + ".tpl"), encoding="UTF-8") as fp:
   code = fp.read()

code = code.replace("$LANG", lang)
if lang != "en" and category == "tokenize":
   code = code.replace("en_assimilation", "void")

sys.stdout.buffer.write(b"/* Generated file, don't edit! */\n")
sys.stdout.buffer.write(code.encode())
