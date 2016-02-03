#!/usr/bin/env python3

"""
Helper for iterating over EOS contexts for training and evaluation. We expect as
input one sentence per line, in the following format:

   Young/LATIN Peter/LATIN Wendell/LATIN ,/SYM a/LATIN student/LATIN

Periods must be separated from the preceding token, even if they aren't
sentence-terminal. Returns a list of tuples of the form:

   ([('maintien', 'LATIN'), ('.', 'SYM'), ('Je', 'LATIN')], 'EOS')
   ([('M', 'LATIN'), ('.', 'SYM'), ('Roseman', 'LATIN')], '!EOS')
"""

import sys, re, unicodedata

# Number of tokens to keep on each side of an EOS marker.
CONTEXT_SIZE = 2

# For which EOS tokens should we emit a context?
BREAK_POINTS = {"."}

ALL_BREAK_POINTS = {".", "?", "!"}

# Dummy token to mark the beginning and the end of the text. Must be kept in
# sync with the C code. We use the empty token because this makes things easier
# on the C side.
PADDING = [("", "UNK", "!EOS")] * CONTEXT_SIZE

def iter_lines(fp):
   for line in fp:
      line = line.strip()
      if line:
         line = re.sub(" +", " ", line)
         line = unicodedata.normalize("NFKC", line)
         yield line

def read_token(itor):
   token = []
   type = None
   while True:
      try:
         i, pair = next(itor)
      except StopIteration:
         return None, None, None
      chunks = pair.rsplit("/", 1)
      token.append(chunks[0])
      if len(chunks) == 2:
         type = chunks[1]
         return i, " ".join(token), type

def tokenize_line(line):
   # Position of the abbreviation that ends the sentence, if any.
   terminal_abbr = -1
   sent = []
   itor = enumerate(line.split())
   while True:
      i, token, type = read_token(itor)
      if not token:
         break
      # In some corpora, the terminal period is duplicated if part of an
      # abbreviation:
      #     Foobar Ltd. .
      # In this case, our tokenizer interprets . . as a single token, so fix
      # that.
      if token == ". .":
         token = "."
         assert terminal_abbr < 0
         terminal_abbr = i
      sent.append((token, type))
   return sent

def find_sent_end(sent):
   """Look for trailing punctuation after a break point at the end of a
   sentence. In this case, make the break point terminal, even if it doesn't
   actually ends the sentence. This is necessary to deal with sentences like:

      The market is just becoming more efficient . "
      { The court has indicated it will rule on the case by the end of the month . }
      ( It is , of course , printed on recycled paper . )
   """
   end = len(sent) - 1
   while end and sent[end][1] == "SYM" and sent[end][0] not in ALL_BREAK_POINTS:
      end -= 1
   if sent[end][0] not in ALL_BREAK_POINTS:
      end = len(sent) - 1
   return end

def read_tokens(fp):
   tokens = PADDING.copy()
   for line in iter_lines(fp):
      sent = tokenize_line(line)
      end = find_sent_end(sent)
      for token, tag in sent[:end]:
         tokens.append((token, tag, "!EOS"))
      for token, tag in sent[end:end + 1]:
         tokens.append((token, tag, "EOS"))
      for token, tag in sent[end + 1:]:
         tokens.append((token, tag, "!EOS"))
   tokens.extend(PADDING)
   return tokens

def read_contexts(fp, allow_empty_features=True):
   def iter_contexts(tokens):
      for i, (token, _, sent_break) in enumerate(tokens):
         if token not in BREAK_POINTS:
            continue
         pairs = []
         for token, tag, _ in tokens[i - CONTEXT_SIZE:i + CONTEXT_SIZE + 1]:
            # bayes_fss doesn't accept empty features.
            if not token and not allow_empty_features:
               token = "***BOUNDARY***"
            pairs.append((token, tag))
         yield pairs, sent_break

   return list(iter_contexts(read_tokens(fp)))

if __name__ == "__main__":
   for c in read_contexts(sys.stdin):
      print(c)
