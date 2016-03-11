%%{

# Extended grapheme clusters recognition. For details, see:
# http://unicode.org/reports/tr29/#Grapheme_Cluster_Boundaries
# This works on (valid) UTF-8 sequences. The alphabet type must be set to
# unsigned char.

machine grapheme;

include grapheme_break_property "grapheme_break_property.rl";

code_point = (0x00 .. 0x7F)
           | (0xc0 .. 0xdf) any
           | (0xe0 .. 0xef) any any
           | (0xf0 .. 0xf7) any any any
           ;

Not_Control = code_point - Control - CR - LF;

RI_Sequence = Regional_Indicator+;

Hangul_Syllable =
  L* V+ T*
| L* LV V* T*
| L* LVT T*
| L+
| T+
;
 
grapheme =
  CR LF
| Prepend* (RI_Sequence | Hangul_Syllable | Not_Control) (Extend | SpacingMark)*
| Control | CR | LF
;

}%%
