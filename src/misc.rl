/* Recognition of special tokens types. */

%%{

machine misc;

include utf8 "utf8.rl";

# Abbreviation.
#
# Note that this pattern overlaps with the Latin word pattern; it must be placed
# above it in the scanner definition to take precedence.
#
# Also note that we don't try to match a terminal period, because periods are
# ambiguous. Disambiguation must be done afterwards, in the SBD module.

abbreviation = latin_letter ("." latin_letter)+;

# Email.
#
#     foobar@qux.com
#     café@café.coiffé
#
# Not all email domains have an extension (.net, .com, etc.). But not requiring
# the extension would make the pattern too general.

email = (code_point+ -- whitespace) "@" (code_point+ -- whitespace) "." latin_letter+;

# URI.
#
#     file:///home/foobar/
#     www.foobar.com
#     smb://hostname/directorypath/resource
#
# URI of the form www.foo.com overlap with the abbreviation pattern, and must
# then be put above it in the scanner definition. The "www." and "ftp."
# prefixes are relatively ambiguous, so we use them here; being too general
# would prevent proper recognition of abbreviations.

uri = ("www."i | "ftp."i) (code_point+ -- whitespace) "." alpha+
    | alpha+ "://" (code_point+ -- whitespace) (alnum | "/")
    ;

# Path in the file system. We don't care about Windows.
#
#    /usr/bin/foobar
#    ~/foobar
#    ./foobar
#    ../fobar
#
# A specific pattern must be defined because otherwise the leading / or ~/
# would be split apart from the path.
#
# We require that the root path is a known directory to reduce noise. In
# addition, the last letter of the path must be alphanumeric, to avoid including
# punctuation (closing bracket, etc.).
#
# Of the machines below, only "path" is public.

dir_at_root = "bin" | "boot" | "dev" | "etc" | "home" | "lib" | "media" | "mnt"
            | "opt" | "proc" | "root" | "run" | "sbin" | "srv" | "sys" | "tmp"
            | "usr" | "var"
            ;

home_path = ("~" | "."{1,2}) "/" (code_point+ -- whitespace) latin;

root_path = "/" dir_at_root ("/" (code_point+ -- whitespace) latin)?;

path = (home_path | root_path) "/"?;

# Everything but one of the known token types.
#
# This includes words not written in Latin notation. We try to match the longest
# possible sequence of unknown characters so that we at least have a text
# segment that could be a word. This works well in practice when a foreign
# script is mixed with predominantly Latin text.

unknown = code_point+ -- (symbol | latin | whitespace);

}%%
