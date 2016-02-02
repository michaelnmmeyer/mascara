import unicodedata
from context import CONTEXT_SIZE

# Keep that in sync with the C code.
MAX_FEATURE_LEN = 128
MAX_FEATURES = 0xff - 1

FEATURE_JOIN_STRING = "+"
VALUE_JOIN_STRING = "+"

VOWELS = "aeiouyáéíóúàèìòùäëïöüâêîôûāēīōū"
VOWELS += VOWELS.upper()
VOWELS = frozenset(unicodedata.normalize("NFC", VOWELS))

# Features to try:
# *   Use a string around the period, without using explicit token boundaries.
#     Requires some preprocessing (remove whitespace, etc.)
# *   Use a lexicon tag.

def ft_len(token):
   # Using bytes actually improves performance (and this is faster in C).
   l = len(token.encode("UTF-8"))
   if l == 0:
      return "0"
   if l == 1:
      return "1"
   if l <= 3:
      return "2..3"
   if l <= 5:
      return "4..5"
   if l <= 7:
      return "6..7"
   return "8.."

def ft_case(token):
   if not token or token[0].isupper():
      return "LCAP"
   return "LLOW"
  
def ft_shape(token, tag):
   # Tried to map "PREFIX" and "SUFFIX" to "LATIN", decreases performance.
   if tag == "LATIN":
      return ft_case(token)
   return tag

def ft_mask(token):
   mask = ""
   for c in token:
      if c in VOWELS:
         mask += "V"
      elif c.isalpha():
         mask += "C"
      elif c.isdigit():
         mask += "D"
      else:
         mask += "P"
   return mask

def ft_unimask(token):
   mask = ""
   for c in token:
      mask += unicodedata.category(c)
   return mask

EXTRACTORS = {
   "l_word": lambda ctx: ctx[CONTEXT_SIZE-1][0],
   "r_word": lambda ctx: ctx[CONTEXT_SIZE+1][0],
   "l_len": lambda ctx: ft_len(ctx[CONTEXT_SIZE-1][0]),
   "r_len": lambda ctx: ft_len(ctx[CONTEXT_SIZE+1][0]),
   "l_suffix4": lambda ctx: ctx[CONTEXT_SIZE-1][0][-4:],
   "l_suffix3": lambda ctx: ctx[CONTEXT_SIZE-1][0][-3:],
   "l_suffix2": lambda ctx: ctx[CONTEXT_SIZE-1][0][-2:],
   "r_prefix2": lambda ctx: ctx[CONTEXT_SIZE+1][0][:2],
   "r_prefix3": lambda ctx: ctx[CONTEXT_SIZE+1][0][:2],
   "r_prefix4": lambda ctx: ctx[CONTEXT_SIZE+1][0][:4],
   "l_shape": lambda ctx: ft_shape(*ctx[CONTEXT_SIZE-1]),
   "r_shape": lambda ctx: ft_shape(*ctx[CONTEXT_SIZE+1]),
   "l_case": lambda ctx: ft_case(ctx[CONTEXT_SIZE-1][0]),
   "r_case": lambda ctx: ft_case(ctx[CONTEXT_SIZE+1][0]),
   "l_mask": lambda ctx: ft_mask(ctx[CONTEXT_SIZE-1][0]),
   "r_mask": lambda ctx: ft_mask(ctx[CONTEXT_SIZE+1][0]),
   "l_unimask": lambda ctx: ft_unimask(ctx[CONTEXT_SIZE-1][0]),
   "r_unimask": lambda ctx: ft_unimask(ctx[CONTEXT_SIZE+1][0]),
}

def compound_extractor(feat_names):
   if isinstance(feat_names, str):
      return feat_names, EXTRACTORS[feat_names]
   def extract(ctx):
      return VALUE_JOIN_STRING.join(EXTRACTORS[name](ctx) for name in feat_names)
   return FEATURE_JOIN_STRING.join(feat_names), extract
