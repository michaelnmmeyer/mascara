/* Generated file, don't edit! */
%%{

machine en_suffix_match;

include en_suffix "en_suffix.rl";

alphtype unsigned char;

main := |*
   en_suffix => { return ts + 1 - p; };
*|;

}%%

%% write data noerror nofinal;

/* Finds the longest known suffix of a string.
 * "p" must point to the last byte in the string to examine.
 * "pe" must point to the first character of the string to examine.
 *
 * We require that a suffix doesn't match the whole string; in other words, the
 * string must have a leading part of length >= 1. This is required both because
 * we don't want a suffix to be recognized as such when it doesn't occur
 * immediately after a word, and because there is an ambiguity when a token has
 * both a prefix and a suffix but the prefix and the suffix overlap, e.g.:
 *
 *    C’s de facto packaging system
 *
 * Here C’ is a valid French prefix, and ’s is a valid English suffix. There is
 * no easy way to decide which segmentation is the best one, so we choose
 * (arbitrarily) to emit only the prefix, and treat the rest of the string as a
 * single token.
 *
 * If a suffix is found, returns its length, otherwise zero.
 */
static size_t en_suffix(const unsigned char *p, const unsigned char *pe)
{
   int cs, act;
   const unsigned char *ts, *te;
   const unsigned char *const eof = pe;

   %% write init;
   %% write exec;

   return 0;

   /* Unused variables. */
   (void)act;
   (void)te;
   (void)eof;
   
   (void)en_suffix_match_en_main;
}
