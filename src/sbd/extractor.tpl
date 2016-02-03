/* Generated code, don't edit! */

local const char *const {{name}}_features[] = {
% for fs in features:
   "{{FEATURE_JOIN_STRING.join(fs)}}",
% end
   NULL
};

local bool {{name}}_at_eos(const struct bayes *mdl,
                            const struct mr_token *l, const struct mr_token *r)
{
   double vec[2];
   char stack[1 + MAX_FEATURE_LEN * {{max_compound_features}} + {{max_compound_features}}], *buf;

   bayes_init(mdl, vec);

% for feat_no, fs in enumerate(features, 1):
   buf = stack;
   *buf++ = {{feat_no}};
% for sub_feat_no, sub_feat in enumerate(fs, 1):
   buf = mr_ft_{{sub_feat.split("_")[1]}}(buf, {{sub_feat.split("_")[0]}});
   *buf++ = '{{sub_feat_no < len(fs) and VALUE_JOIN_STRING or "\\0"}}';
% end
   bayes_feed(mdl, vec, stack);

% end
   return vec[EOS] >= vec[NOT_EOS];
}

local const struct sentencizer2_config {{name}}_config = {
   .bayes_config = {
      .name = "{{name}}",
      .version = {{version}},
      .features = {{name}}_features,
   },
   .at_eos = {{name}}_at_eos,
};
