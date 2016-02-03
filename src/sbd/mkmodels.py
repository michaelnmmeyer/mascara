#!/usr/bin/env python3

import os, sys, json, math, hashlib, bottle
from collections import defaultdict
import context, features as ftrs

SMOOTH = 0.5
CORPORA_DIR = "data"

def train(labeled_features):
   label_freqs = defaultdict(int)
   cond_freqs = defaultdict(lambda: defaultdict(int))
   features = defaultdict(set)
   for fs, label in labeled_features:
      label_freqs[label] += 1
      for fname, fval in fs.items():
         assert len(fval.encode("UTF-8")) <= ftrs.MAX_FEATURE_LEN
         cond_freqs[fname, fval][label] += 1
         features[fname].add(fval)

   # Priors
   priors = {}
   for label, label_freq in label_freqs.items():
      prob = (label_freq + SMOOTH) / (sum(label_freqs.values()) + SMOOTH * len(label_freqs))
      priors[label] = math.log2(prob)

   # Unk. probs.
   unk_probs = {}
   for fname, fvals in features.items():
      probs = {}
      for label, label_freq in label_freqs.items():
         prob = SMOOTH / (label_freq + SMOOTH * len(fvals))
         probs[label] = math.log2(prob)
      unk_probs[fname] = probs

   # Cond. probs.
   cond_probs = {}
   for (fname, fval), freqs in cond_freqs.items():
      probs = {}
      for label, label_freq in label_freqs.items():
         freq = freqs.get(label, 0)
         prob = (freq + SMOOTH) / (label_freq + SMOOTH * len(features[fname]))
         probs[label] = math.log2(prob)
      cond_probs[(fname, fval)] = probs

   return priors, unk_probs, cond_probs

def encode_string(s):
   assert isinstance(s, str) and not "\0" in s
   return "%d:%s" % (len(s.encode("UTF-8")), s)

def sha1(s):
   return hashlib.sha1(s.encode("UTF-8")).hexdigest()

"""
Model file format:

mr_bayes 1
<model_name> <sha1>
EOS <eos_prior> !EOS <not_eos_prior>
features <num_features> values <num_values>
<feature_name> <eos_unknown_prob> <not_eos_unknown_prob>
...
<feature_no> <feature_value> <eos_prob> <not_eos_prob>
...
"""
def dump_model(name, features, ctxs, fp):
   assert len(features) <= ftrs.MAX_FEATURES
   extractors = [ftrs.compound_extractor(f) for f in features]

   def labeled_features():
      for ctx, label in ctxs:
         yield {name: fn(ctx) for name, fn in extractors}, label

   priors, unk_probs, cond_probs = train(labeled_features())
   assert len(priors) == 2

   dummy_version = sha1("foobar")
   buf = []
   p = lambda s: buf.append("%s\n" % s)
   
   p("mr_bayes 1")
   p("%s %s" % (name, dummy_version))
   p("features %d values %d" % (len(unk_probs), len(cond_probs)))
   p("EOS %s !EOS %s" % (priors["EOS"].hex(), priors["!EOS"].hex()))
   feat_names = sorted(unk_probs, key=lambda s: s.encode("UTF-8"))
   for fname in feat_names:
      peos, pneos = unk_probs[fname]["EOS"], unk_probs[fname]["!EOS"]
      p("%s %s %s" % (encode_string(fname), peos.hex(), pneos.hex()))
   for (fname, fval), probs in sorted(cond_probs.items()):
      feat_no = feat_names.index(fname)
      fval = encode_string(fval)
      peos, pneos = probs["EOS"], probs["!EOS"]
      p("%s %s %s %s" % (feat_no, fval, peos.hex(), pneos.hex()))
   
   buf = "".join(buf)
   version = sha1(buf)
   buf = buf.replace(dummy_version, version)
   fp.write(buf)
   return version   

def dump_code(name, version, features, cout):
   max_compound_features = max(len(f) for f in features)
   params = {
      "max_compound_features": max_compound_features,
      "name": name,
      "version": version,
      "features": features,
      "FEATURE_JOIN_STRING": ftrs.FEATURE_JOIN_STRING,
      "VALUE_JOIN_STRING": ftrs.VALUE_JOIN_STRING,
   }
   with open("extractor.tpl", "r") as fp:
      code = bottle.template(fp.read(), **params)
   print(code.strip(), file=cout)

def mkmodel(config):
   this_dir = os.path.dirname(os.path.abspath(__file__))
   root_dir = os.path.dirname(os.path.dirname(this_dir))
   
   name = config["name"]
   features = [isinstance(f, str) and [f] or f for f in config["features"]]
   features.sort(key=lambda fs: ftrs.FEATURE_JOIN_STRING.join(fs).encode("UTF-8"))

   with open(os.path.join(CORPORA_DIR, name + ".tok")) as fp:
      ctxs = context.read_contexts(fp)
   with open(os.path.join(root_dir, "models", name + ".mdl"), "w") as fp:
      version = dump_model(name, features, ctxs, fp)
   with open(os.path.join(root_dir, "src", name + ".cm"), "w") as fp:
      dump_code(name, version, features, fp)

if __name__ == "__main__":
   path = os.path.join(os.path.dirname(__file__), "config.json")
   with open(path) as fp:
      for config in json.load(fp):
         print(config["name"])
         mkmodel(config)
