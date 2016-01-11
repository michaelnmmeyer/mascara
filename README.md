# mascara

A tokenizer for English and French.

## Purpose

Penn Treebank-style tokenization for English and French.

## Building

The library is available in source form, as an amalgamation. Compile `mascara.c`
together with your source code, and use the interface described in `mascara.h`.
A C11 compiler is required for compilation, which means either GCC or CLang on
Unix.

A command-line tool `mascara` is included. To compile and install it:

    $ make && sudo make install


## Usage

The API is described in `mascara.h`. There is a concrete example in
`example.c`. Use the corresponding binary like so:

    ./example "And now, Laertes, what's the news with you?"

During tokenization, each token is annotated with a type. This information is
sometimes useful for its own sake, but it is intended to be used as feature for
later processing. Existing token types are:

*  `LATIN`. A token principally made of Latin characters:

       Hamlet
       entr'ouvert
       willy-nilly
       AT&T
       tris(dimethylamino)bromophosphonium

*  `ELISION`. An elision at the beginning of a text segment:

       y'  => y'know
       d'  => d'entrée de jeu
       qu' => qu'on se le dise

*  `SUFFIX`. A token at the end of a text segment:

       'll   => he'll
       'd    => he'd
       -t-il => pense-t-il

*  `SYM`. A symbol. This doesn't include all Unicode symbols, only the most
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

* `EMAIL`. An email address:

      foo@example.com
      john@café.be

* `URI`. A likely URI:

      http://www.example.com?q=fubar
      www.google.de
      
 * `PATH`. A path in the file system:
 
       /usr/bin/fubar
       ~/home_sweet_home/foo.txt
 
* `UNK`. Anything but one of the above. This includes unknown symbols, as well as words not in the Latin script. The longest possible span of unknown characters is systematically selected:

      ☎
      मन्त्र

You can check wich type is assigned to which token with the command-line tool:

    $ echo "And now, Laertes, what's the news with you?" | mascara -f "%s/%t "
    And/LATIN now/LATIN ,/SYM Laertes/LATIN ,/SYM what/LATIN 's/SUFFIX the/LATIN
    news/LATIN with/LATIN you/LATIN ?/SYM

