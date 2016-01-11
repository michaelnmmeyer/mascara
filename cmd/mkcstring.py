#!/usr/bin/env python3

"""
Reads a file from stdin and outputs it as a C string, suitable for embedding
into a C source file.
"""

import sys

replacements = {
	"\n": "n",
	"\t": "t",
	"\\": "\\",
	"\"": '"',
}

for key, value in replacements.copy().items():
	replacements[key] = "\\" + value

for line in sys.stdin:
	print('"%s"' % "".join(replacements.get(c, c) for c in line))
