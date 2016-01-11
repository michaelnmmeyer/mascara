/* Recognition of latin characters. */
%%{

machine latin;

include raw_latin_letter "gen/raw_latin_letter.rl";
include combining_diacritic "combining_diacritic.rl";

# The following is not strictly valid, but works in practice.
latin_letter = raw_latin_letter combining_diacritic*;

latin = latin_letter | [0-9];

}%%
