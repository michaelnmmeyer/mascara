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

dash =
  "-"
| "–"     # EN DASH
| "—"     # EM DASH
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
