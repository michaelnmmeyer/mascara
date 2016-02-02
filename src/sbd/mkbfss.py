#!/usr/bin/env python3

"""
Reads a corpus from the standard input (see context.py for the expected format),
extracts features from it, and output a dataset file for use with bayes_fss.
"""

import sys, features, context

ctxs = context.read_contexts(sys.stdin, allow_empty_features=False)
fns = sorted(features.EXTRACTORS.items())

print("\t" + "\t".join(name for name, _ in fns))
for ctx, label in ctxs:
   fields = [fn(ctx) for _, fn in fns]
   print("%s\t%s" % (label, "\t".join(fields)))
