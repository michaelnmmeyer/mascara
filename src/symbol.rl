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

double_quote =
  '"'
| "“"
| "”"
| "„"
| "«"
| "»"
| "‹"
| "›"
| "``"   # Latex-style
| "''"
| "<<"   # Some people use this instead of real guillemets.
| ">>"
;

dash =
  "-"
| "–"     # EN DASH
| "—"     # EM DASH
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
| "…"
| "."+
| "." (" .")+   # . . .
| "?"+
| "!"+

# Single quote.
| apostrophe
| "‚"    # U+201A, not a comma!

| double_quote

# Brackets.
| "("
| ")"
| "["
| "]"
| "{"
| "}"

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
