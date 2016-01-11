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

Use it like so:

    echo "And now, Laertes, what's the news with you?" | mascara

## Usage

See the documentation in `mascara.h`. There is a concrete usage example in
`example.c`. Use the corresponding binary like so:

    ./example "And now, Laertes, what's the news with you?"
