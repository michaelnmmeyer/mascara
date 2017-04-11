%%{

# Extended grapheme clusters recognition (Unicode 9.0.0).
#
# For details, see:
# http://unicode.org/reports/tr29/#Grapheme_Cluster_Boundaries
#
# This works on (valid) UTF-8 sequences. The alphabet type must be set to
# unsigned char.

machine grapheme;

include grapheme_properties "grapheme_properties.rl";

Not_Control = Code_Point - Control - CR - LF;

Hangul_Sequence =
  L* V+ T*
| L* LV V* T*
| L* LVT T*
| L+
| T+
;

Emoji_Sequence =
  (E_Base | E_Base_GAZ) Extend* E_Modifier
| ZWJ (Glue_After_Zwj | E_Base_GAZ)
| ZWJ E_Base_GAZ Extend* E_Modifier
;

# There is an issue with rules WB15 and WB16. See :
#    http://unicode.org/reports/tr29/#WB15.
# I'd understand : "match 1, 2, 4, 6... RI ; prefer the longest possible
# sequence." But then the test file has, at line 835 :
#   ÷ 0061 ÷ 1F1E6 × 1F1E7 ÷ 1F1E8 × 1F1E9 ÷ 0062
# I take this to mean : "match either 1 or 2 RI, not more".
RI_Sequence = Regional_Indicator{2};

Sequence = Hangul_Sequence | Emoji_Sequence | RI_Sequence;
 
grapheme =
  CR LF
| Prepend* (Sequence | Not_Control) (Extend | SpacingMark | ZWJ)*
| Control | CR | LF
;

}%%
