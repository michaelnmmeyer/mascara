/* Recognition of code points encoded in UTF-8. */

%%{

machine utf8;

# We can't use directly the builtin "any" machine to match spans of arbitrary
# characters because substracting multi-byte characters like so:
#
#    (any+ -- "”")
#
# leads to invalid UTF-8 sequences. For example, applying the above to the
# string #” yields two tokens:
#
#     # and the first two bytes of ”
#     the third byte of ”
#
# Still, we don't check here whether we have a correct UTF-8 sequence, but
# merely look at the first byte to determine the length of the sequence.
# UTF-8 validation must be done beforehand.

code_point = (0x00 .. 0x7F)
           | (0xc0 .. 0xdf) any
           | (0xe0 .. 0xef) any any
           | (0xf0 .. 0xf7) any any any
           ;
}%%
