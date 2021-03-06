-- Don't emit anything if there is no input.
check{
   input = "",
   output = {},
}

-- Emit the last sentence in the input text even if not followed by an explicit
-- terminator.
check{
   input = "foo",
   output = {
      {"foo"}
   },
}

-- Recognize Unicode periods, and reattach them correctly when they're part of
-- an abbreviation.
check{
   input = "Ah! princesse﹒ Hello M﹒ Johnes﹒",
   lang = "fr",
   output = {
      {"Ah", "!", "princesse", "﹒"},
      {"Hello", "M﹒", "Johnes", "﹒"},
   },
}

-- Always emit a separate period if at the end of the input text. We're sure
-- there is a sentence ending in this case.
check{
   input = "Mr. Mr.",
   lang = "en",
   output = {
      {"Mr.", "Mr", "."},
   },
}
check{
   input = "Mr﹒ Mr﹒  ",
   lang = "en",
   output = {
      {"Mr﹒", "Mr", "﹒"},
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
-- Reattach periods at the end of an ordinal.
check{
   input = "Am 31. Dezember",
   output = {{"Am", "31.", "Dezember"}},
   lang = "de",
   impl = "bayes",  -- Not recognized as sentence boundary by the FSM.
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

-- Don't split if a comma, etc., follows:
check{
   input = "foo., bar",
   output = {{"foo.", ",", "bar"}},
   impl = "fsm",
}

-- Special case
check{
   input = "(und vor Gelons Tod 216 v﹒ Chr﹒)﹒ Er widerlegte",
   output = {
      {"(", "und", "vor", "Gelons", "Tod", "216", "v﹒", "Chr﹒", ")", "﹒"},
      {"Er", "widerlegte"},
   },
   impl = "fsm",  -- Doesn't concern the classifier.
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

-- Don't split if not followed by an uppercase letter.
check{
   input = "foo. bar",
   output = {{"foo.", "bar"}},
}

-- Include trailing quotes at the end of a sentence in the sentence itself.
check{
   input = [[
      «C'est vrai, répondait mon amour, il n'y a plus rien à faire de cette
      amitié-là, elle ne changera pas.»
   ]],
   lang = "fr",
   output = {
      {"«", "C'", "est", "vrai", ",", "répondait", "mon", "amour", ",", "il",
      "n'", "y", "a", "plus", "rien", "à", "faire", "de", "cette", "amitié",
      "-là", ",", "elle", "ne", "changera", "pas", ".", "»"},
   }
}

-- Don't split on abbreviations.
check{
   input = "U.S. laws alone swamp our small staff.",
   output = {{"U.S.", "laws", "alone", "swamp", "our", "small", "staff", "."}}
}
check{
   input = "seit M.-S. Samain",
   lang = "de",
   impl = "fsm",  -- Not recognized as sentence boundary by the classifier.
   output = {{"seit", "M.-S.", "Samain"}},
}
check{
   input = "der H.-G.-Wells-Literaturverfilmung",
   output = {{"der", "H.-G.-Wells-Literaturverfilmung"}},
}
check{
   input = "(und vor Gelons Tod 216 v. Chr.). Er widerlegte",
   output = {
      {"(", "und", "vor", "Gelons", "Tod", "216", "v.", "Chr.", ")", "."},
      {"Er", "widerlegte"},
   }
}

-- Don't split on p. 1234.
check{
   input = "See p. 5 for more informations.",
   output = {{"See", "p.", "5", "for", "more", "informations", "."}}
}

-- Split sentences that would grow too much.
local max_sent_len = require("mascara").MAX_SENTENCE_LEN
local output = {}
for i = 1, max_sent_len do
   output[i] = "foo"
end
check{
   input = table.concat(output, " ") .. " foo",
   output = {output, {"foo"}},
}
