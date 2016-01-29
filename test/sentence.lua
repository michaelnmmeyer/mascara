-- Emit the last sentence in the input text even if not followed by an explicit
-- terminator.
check{
   input = "foo",
   output = {
      {"foo"}
   },
}

-- Reattach periods correctly.
check{
   input = [[
      He said: "Mr. and Mrs. Smith have two children".
   ]],
   output = {
      {"He", "said", ":", '"', "Mr.", "and", "Mrs.", "Smith",
       "have", "two", "children", '"', "."}
   },
}

-- Recognize paragraph breaks when appropriate.
check{
   input = "foo\nbar",
   output = {{"foo", "bar"}},
}
check{
   input = "foo\n\nbar",
   output = {{"foo"}, {"bar"}},
}
check{
   input = "foo\r\nbar",
   output = {{"foo", "bar"}},
}
check{
   input = "foo\r\n\nbar",
   output = {{"foo"}, {"bar"}},
}

-- Special case: '!' + lowercase.
check{
   input = [[
      Ah! princesse.
      "Good gracious!" cried Mrs. Bennet.
      "Why?" said Albert.
   ]],
   output = {
      {"Ah", "!", "princesse", "."},
      {'"', "Good", "gracious", "!", '"', "cried", "Mrs.", "Bennet", "."},
      {'"', "Why", "?", '"', "said", "Albert", "."},
   }
}

-- Don't split on URLs.
check{
   input = "To donate, please visit: http://pglaf.org/donate Section 5. Foobar",
   output = {
      {"To", "donate", ",", "please", "visit", ":", "http://pglaf.org/donate", "Section", "5", "."},
      {"Foobar"}
   }
}

-- Don't split on initials.
check{
   input = "Professor Michael S. Hart is the originator of the Project Gutenberg. Foobar",
   output = {
      {"Professor", "Michael", "S.", "Hart", "is", "the", "originator", "of", "the", "Project", "Gutenberg", "."},
      {"Foobar"}
   }
}

-- Don't split on email addresses.
check{
   input = "Gregory B. Newby Chief Executive and Director gbnewby@pglaf.org Section 4. Foobar",
   output = {
      {"Gregory", "B.", "Newby", "Chief", "Executive", "and", "Director", "gbnewby@pglaf.org", "Section", "4", "."},
      {"Foobar"}
   }
}

-- Include trailing quotes at the end of a sentence in the sentence itself.
check{
   input = [[
      «C'est vrai, répondait mon amour, il n'y a plus rien à faire de cette
      amitié-là, elle ne changera pas.»
   ]],
   output = {
      {"«", "C'est", "vrai", ",", "répondait", "mon", "amour", ",", "il", "n'y",
       "a", "plus", "rien", "à", "faire", "de", "cette", "amitié-là", ",",
       "elle", "ne", "changera", "pas", ".", "»"},
   }
}

-- Don't split on abbreviations.
check{
   input = "U.S. laws alone swamp our small staff.",
   output = {{"U.S.", "laws", "alone", "swamp", "our", "small", "staff", "."}}
}

-- Don't split on p. 1234.
check{
   input = "See p. 5 for more informations.",
   output = {{"See", "p.", "5", "for", "more", "informations", "."}}
}

-- Split sentences that would grow too much.
local max_sent_len = require("mascara").MAX_SENTENCE_LEN
local output = {}
for i = 1, max_sent_len - 1 do
   output[i] = "foo"
end
output[max_sent_len] = "foo"
check{
   input = table.concat(output, " ") .. " foo",
   output = {output, {"foo"}},
}
