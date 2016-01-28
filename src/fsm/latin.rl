/* Recognition of latin characters. */
%%{

machine latin;

include raw_latin_letter "raw_latin_letter.rl";
include raw_latin_uppercase "raw_latin_uppercase.rl";
include raw_latin_lowercase "raw_latin_lowercase.rl";
include combining_diacritic "combining_diacritic.rl";

# The following is not strictly valid, but works in practice.
latin_letter = raw_latin_letter combining_diacritic*;
latin_uppercase = raw_latin_uppercase combining_diacritic*;
latin_lowercase = raw_latin_lowercase combining_diacritic*;

latin = latin_letter | [0-9];

}%%
