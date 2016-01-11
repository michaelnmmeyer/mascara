/* Recognition of numeric tokens. */

%%{

machine numeric;

include whitespace "whitespace.rl";

# Generic number.
#
#    10 000
#    30,000
#    34.5
#    -23
#    +3
#    6e-3
#
# This partly overlaps with the phone number regex below, but we don't care at
# this point. Disambiguation should be done with a specific module. Here, we are
# merely interested in knowing that the token is some kind of number.

generic_number = ("-" | "+")? digit+ (("." | "," | horizontal_whitespace+) digit+)* ("e"i ("-" | "+")? digit+)?;

# Decade.
#
#    1980's
#    80's
#
# A specific rule must be defined for this because otherwise 's would be
# split apart.

decade = ("19" | "20") digit "0" apostrophe "s"i
       | [1-9] "0" apostrophe "s"i
       ;

# Phone number.
# Looked at http://stdcxx.apache.org/doc/stdlibug/26-1.html
#
#     +1-541-754-3010
#     (01) 55 1234 5678
#

phone_number = ("+" digit+ horizontal_whitespace?)? (("-" | "/")? (digit+ | ("(" digit+ ")")) horizontal_whitespace?)+ digit+;

# Hexadecimal.
#
# These are tokenized correctly per default (but as LATIN words), so the
# following is not strictly necessary.

hexadecimal = "0x"i (digit | [a-f] | [A-F])+;

# Other number formats.
#
#     20:30
#     1906–07
#     1850-1930
#     pp. 123-27
#
# To match correctly 1906–07, we should also check that 06 < 07, but this is
# difficult to do here. The expression is ambiguous if we have 1906-07 (could
# be a substraction).

number_other = digit+ (":" | dash) digit+;

# Finally, all of the above except generic_number and roman_number, which
# require a special treatment for normalization.

numeric = generic_number | decade | phone_number | hexadecimal | number_other;

}%%
