/* Recognition of symbols.
 * The goal is not to recognize every possible symbol, merely the ones that
 * need to be for performing sensible tokenization.
 */
%%{

# We allow several contiguous occurrences of *, -, + and = because this is
# necessary in some cases:
#
#     **    exponentiation
#     --    decrement, or ASCII representation of EM dash
#     ++    increment
#     ==    comparison
#
# The above are also often used for delimiting text sections in plain text
# markup ala Markdown.

machine symbol;

# We define separately "apostrophe" and "double_quote" because they are
# needed for the main machine.

# Keep that in sync with mklang.py!
apostrophe =
  "'"
| "‘"    # This one is sometimes used for apostrophes, even if it shouldn't.
| "’"
;

opening_single_quote =
  "'"
| "‘"
;

closing_single_quote =
  "'"
| "’"
;

single_quote =
  opening_single_quote
| closing_single_quote
| "‚"    # U+201A, not a comma!
;

opening_double_quote =
  '"'
| '“'
| "„"
| "«"
| "‹"
| "``"      # Latex-style.
| "<<"      # Some people use this instead of real guillemets.
;

closing_double_quote =
  '"'
| "”"
| "»"
| "›"
| "''"
| ">>"
;

double_quote = opening_double_quote | closing_double_quote;

opening_bracket = "(" | "[" | "{";
closing_bracket = ")" | "]" | "}";

bracket = opening_bracket | closing_bracket;

HYPHEN_MINUS = "-";                    # The one on the keyboard.
HYPHEN = 0xe2 0x80 0x90;
NON_BREAKING_HYPHEN = 0xe2 0x80 0x91;
SMALL_HYPHEN_MINUS = 0xef 0xb9 0xa3;

# Keep that in sync with mklang.py!
hyphen =
  HYPHEN_MINUS
| HYPHEN
| NON_BREAKING_HYPHEN
| SMALL_HYPHEN_MINUS
;

MINUS_SIGN = 0xe2 0x88 0x92;

minus =
  HYPHEN_MINUS
| SMALL_HYPHEN_MINUS
| MINUS_SIGN
;

FIGURE_DASH = 0xe2 0x80 0x92;
EN_DASH = 0xe2 0x80 0x93;
EM_DASH = 0xe2 0x80 0x94;
SMALL_EM_DASH = 0xef 0xb9 0x98;
TWO_EM_DASH = 0xe2 0xb8 0xba;
THREE_EM_DASH = 0xe2 0xb8 0xbb;

dash =
  FIGURE_DASH
| EN_DASH
| EM_DASH
| SMALL_EM_DASH
| TWO_EM_DASH
| THREE_EM_DASH
;

# See http://www.unicode.org/reports/tr29/#ATerm
FULL_STOP = ".";
ONE_DOT_LEADER = 0xe2 0x80 0xa4;
SMALL_FULL_STOP = 0xef 0xb9 0x92;
FULLWIDTH_FULL_STOP = 0xef 0xbc 0x8e;

period =
  FULL_STOP
| ONE_DOT_LEADER
| SMALL_FULL_STOP
| FULLWIDTH_FULL_STOP
;

ellipsis =
  "…"
| "."{2,}
| "." (" ."){2,}   # . . .
;

symbol =

# Punctuation.
  ","
| ";"
| ":"
| "!"
| "?"
| "."

# Ellipsis, repeated punctuation characters.
| ellipsis
| "?"+
| "!"+

| single_quote
| double_quote
| bracket

# Currency.
| "$"
| "£"
| "€"

| "©"

# Dashes.
| hyphen
| "-"+
| dash

# Math.
| "-"
| "+"+
| "*"+    # 2 * 5, 6 ** 3, etc.
| "^"
| "/"+    # 2 / 5, 2 // 5
| "%"
| "&"
| "~"
| "="+
| ">" "="?
| "<" "="?
;

}%%
