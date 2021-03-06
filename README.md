# mascara

A natural language tokenizer.

## Purpose

This is a C library and command-line tool for segmenting written texts into
grapheme clusters, tokens, or sentences. It has specific support for English,
French, Italian and German. A generic tokenizer is also available.


## Building

The library is available in source form, as an
[amalgamation](https://www.sqlite.org/amalgamation.html). Compile `mascara.c`
together with your source code, and use the interface described in
[`mascara.h`](https://github.com/michaelnmmeyer/mascara/blob/master/mascara.h).
You'll need a C11 compiler, which means either GCC or CLang on Unix.

A command-line tool `mascara` is included, plus a set of sentence boundary
detection models. To install all these:

    $ make && sudo make install

Although the library itself is BSD-licensed and can thus be used for free in
commercial software, sentence boundary detection models are derived from corpora
covered by more restrictive licenses. Here are the corpora used for creating
each model:

* `en_amalg`: [Brown corpus](http://clu.uni.no/icame/brown/bcm.html), excerpts
  from the [Wall Street Journal](http://www.cis.upenn.edu/~treebank/home.html),
  [BNC 1000 Gold Trees](http://nclt.computing.dcu.ie/~jfoster/resources/bnc1000.html)
* `fr_sequoia`: [Sequoia corpus](https://www.rocq.inria.fr/alpage-wiki/tiki-index.php?page=CorpusSequoia)
* `de_tiger`: [Tiger corpus](http://www.ims.uni-stuttgart.de/forschung/ressourcen/korpora/tiger.en.html)
* `it_tut`: [Turin University Treebank corpus](http://www.di.unito.it/~tutreeb/)


## Usage

### Examples

The [`examples`](https://github.com/michaelnmmeyer/mascara/tree/master/examples)
directory contains concrete usage examples. Compile these files with `make`, and
use them like so:

* Split a text into extended grapheme clusters:

        $ examples/graphemes हृदय
        हृ
        द
        य


* Split a sentence into tokens:

        $ examples/tokens "And now, Laertes, what's the news with you?"
        And
        now
        ,
        Laertes
        ,
        what
        's
        the
        news
        with
        you
        ?

* Split a text into sentences:

        $ examples/sentences "Pierre Vinken, 61 years old, will join the board
        as a nonexecutive director Nov. 29. Mr. Vinken is chairman of Elsevier
        N.V., the Dutch publishing group."  
        Pierre Vinken , 61 years old , will join the board as a nonexecutive director Nov. 29 .  
        Mr. Vinken is chairman of Elsevier N.V. , the Dutch publishing group .

The library API is fully described in `mascara.h`.

### Tokenization mode

Before allocating a tokenizer, you must choose whether you want to iterate over
tokens or over sentences. Segmentation is slightly different depending on which
mode you choose:

* When iterating over tokens, periods that immediately follow a word are always
  separated from it, even if the token is an abbreviation:

      Mr . and Mrs . Smith have two children .

* When iterating over sentences, all but the last period of the sentence are
  left attached to the token that precedes them, provided it is a word:
  
      Mr. and Mrs. Smith have two children .


### Token types

During tokenization, each token is annotated with a type. This information is
sometimes useful for its own sake, but it is intended to be used as feature for
later processing. Existing token types are:

* `LATIN`. A token principally made of Latin characters:

        Hamlet
        entr'ouvert
        willy-nilly
        AT&T
        tris(dimethylamino)bromophosphonium

* `PREFIX`. A token at the beginning of a text segment:

        y'    => y'know
        d'    => d'entrée de jeu
        qu'   => qu'on se le dise
        dell' => dell'altro

* `SUFFIX`. A token at the end of a text segment:

        'll   => he'll
        'd    => he'd
        -t-il => pense-t-il

* `SYM`. A symbol. This doesn't include all Unicode symbols, only the most
  common ones that need to be recognized for the input text to be tokenized
  correctly:

      ?
      !!!
      +
      $

* `NUM`. A numeric token. This includes numbers in decimal, hexadecimal, and
  exponential notation, phone numbers, and a few other types. Examples:

      1,234,567
      80's
      12.34
      0xdeadbeef
      20 000
      3e-27

* `ABBR`. A likely abbreviation, with internal periods:

        Ph.D.
        a.m.
        J.-C.

* `EMAIL`. An email address:

        foo@example.com
        john@café.be

* `URI`. A likely URI:

        http://www.example.com?q=fubar
        www.google.de
      
* `PATH`. A path in the file system:
 
        /usr/bin/fubar
        ~/home_sweet_home/foo.txt
 
* `UNK`. Anything but one of the above. This includes unknown symbols, as well
  as words not in the Latin script. The longest possible span of unknown
  characters is systematically selected:

      ☎
      मन्त्र

You can check wich type is assigned to which token with the command-line tool:

    $ echo "And now, Laertes, what's the news with you?" | mascara -f "%s/%t "
    And/LATIN now/LATIN ,/SYM Laertes/LATIN ,/SYM what/LATIN 's/SUFFIX the/LATIN
    news/LATIN with/LATIN you/LATIN ?/SYM


## Implementation

### Tokenization

There are two main
approaches for implementing a tokenizer: a) using finite-state automata, and b)
using a supervised sequence model. The second solution is much heavier than its
alternative and doesn't seem to be worth the extra work for such a light task as
tokenization, so I discarded it.

Each tokenizer uses two finite-state machines, written in
[Ragel](http://www.colm.net/open-source/ragel/). The first one matches the input
text from left to right, in the usual way. The second one reads it from right to
left, and is used to recognize contractions at the end of a word. Using two
separate machines helps to disambiguate the role of single quotes.

In my first attempt at the task, I performed a preliminary segmentation of the
text on whitespace characters, and then repeatedly attempted to trim tokens
(punctuation, prefixes, suffixes) from the left and the right of the delimited
text chunks. I changed that because this cannot deal with tokens that contain
internal whitespace characters, such as numbers in French.

### Sentence boundary detection

Two sentence boundary detection modules are implemented.

The first one is a simple finite-state machine. It uses a fixed list of rules
and abbreviations. It cannot disambiguate the most ambiguous use of periods
(cardinal numbers, abbreviation at the end of a sentence, etc.). It is currently
only used as a fallback, when no language-specific model is available.

The second one is a finite-state machine that uses a Naive Bayes classifier to
disambiguate the role of periods. Only periods that are seemingly not
token-internal are examined by the classifier. The remaining ones, as well as
question marks, etc., are deemed to be unambiguous, and are always classified as
end-of-sentence markers. We use one feature set per language, obtained through
semi-automatic optimization with the help of my [feature selection
tool](https://github.com/michaelnmmeyer/bayes_fss). Features are extracted from
a window of three tokens, centered on the period to classify.

Despite the simplicity of this approach, its performance is close to
state-of-the-art. The following tables show the performance of our classifier on
four corpora. We use five-fold cross-validation, and only classify ambiguous
periods.

* Brown

                   accuracy    precision   recall      F1
        bayes      98.83       99.61       99.11       99.35
        baseline   90.83       90.83       100.00      95.19

* Sequoia

                   accuracy    precision   recall      F1
        bayes      98.89       98.94       99.92       99.43
        baseline   96.28       96.28       100.00      98.11

* Tiger

                   accuracy    precision   recall      F1
        bayes      99.48       99.65       99.79       99.72
        baseline   93.34       93.34       100.00      96.58

* Turin University Treebank

                   accuracy    precision   recall      F1
        bayes      98.22       99.53       98.38       98.95
        baseline   85.58       85.58       100.00      92.23

### References

* [Grefenstette and Tapanainen (1994), What is a word, What is a sentence?
  Problems of
  Tokenization](http://www.delph-in.net/courses/07/nlp/Gre:Tap:94.pdf).
* [Gillick (2009), Sentence Boundary Detection and the Problem with the U.S.](http://www.dgillick.com/resource/sbd_naacl_2009.pdf)
* [Maršík and Bojar (2012), TrTok: A Fast and Trainable Tokenizer for Natural
  Languages](https://ufal.mff.cuni.cz/pbml/98/art-marsik-bojar.pdf)
* [Evang et al. (2013), Elephant: Sequence Labeling for Word and Sentence
  Segmentation](http://www.aclweb.org/anthology/D13-1146)
