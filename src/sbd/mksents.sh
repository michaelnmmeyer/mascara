#!/usr/bin/env bash

CORPORA_DIR="data"

# Prepares a corpus for training. There must be one sentence per line. Outputs
# a file containing one sentence per line, with annotated tokens:
#
#    The/LATIN 32/NUM steps/LATIN ./SYM
#
dump() {
   CORPUS=$1
   LANG=$2
   cat $CORPORA_DIR/$LANG'_'$CORPUS.txt | while read sent; do
      echo $sent | mascara -l $LANG -e '' -f '%s/%t '
      echo
   done > $CORPORA_DIR/$LANG'_'$CORPUS.tok
}

dump bnc1000gold en
dump brown en
dump amalg en
dump sequoia fr
dump tiger de
dump treebank en