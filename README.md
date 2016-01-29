# mascara

A tokenizer for English, French and Italian.

## Purpose

This is a C library for performing Treebank-style tokenization.


## Building

The library is available in source form, as an amalgamation. Compile `mascara.c`
together with your source code, and use the interface described in `mascara.h`.
A C11 compiler is required for compilation, which means either GCC or CLang on
Unix.

A command-line tool `mascara` is included. To compile and install it:

    $ make && sudo make install


## Usage

There is a concrete usage example in `example.c`. Compile this file with `make`,
and use the output binary like so:

    $ ./example "And now, Laertes, what's the news with you?"

The library API is described in `mascara.h`. The following are auxiliary notes.


## Tokenization mode

Before allocating a tokenizer, you must choose whether you want to iterate over
tokens or over sentences. Segmentation is slightly different depending on which
mode you choose:

* When iterating over tokens, periods that immediately follow a word are always
  separated from it, even if the token is an abbreviation:

      Mr . and Mrs . Smith have two children .

* When iterating over sentences, all but the last period of the sentence are
  left attached to the token that precedes them, provided it is a word:
  
      Mr. and Mrs. Smith have two children .


## Token types

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

From what I gathered by reading the relevant literature, there are two main
approaches for implementing a tokenizer: a) using finite-state automata, and b)
using a supervised sequence model. The second solution is much heavier than its
alternative and doesn't seem to be worth the extra work for such a light task as
tokenization, so I discarded it.

Each tokenizer uses two finite-state machines, written in
[Ragel](http://www.colm.net/open-source/ragel/). The first one matches the input
text from left to right, in the usual way. The second one reads it from right to
left, and is used to recognize contractions at the end of a word. Using two
separate machines helps to disambiguate the role of single quotes.
