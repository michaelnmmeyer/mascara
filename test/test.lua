local json = require("json")
local mascara = require("mascara")

local function check_fail(expected, output)
   local caller_caller = assert(debug.getinfo(3))
   print(string.format("fail at line %d:", caller_caller.currentline))
   print("== Expected:")
   print(json.stringify(expected))
   print("== Have:")
   print(json.stringify(output))
   error("Fail!")
end

local function check(test)
   local tokens = {}
   local format = test.format or {"str"}
   if type(format) == "string" then
      format = {format}
   end
   local tok_tab = {}
   local mr = mascara.new(test.lang or "en")
   mr:set_text(test.input)
   while mr:next_token(tok_tab) do
      -- print(tok_tab.str, tok_tab.type, tok_tab.offset)
      if #format == 1 then
         local attr = format[1]
         table.insert(tokens, assert(tok_tab[attr]))
      else
         local tok = {}
         for _, attr in ipairs(format) do
            table.insert(tok, assert(tok_tab[attr]))
         end
         table.insert(tokens, tok)
      end
   end
   if #tokens ~= #test.output then check_fail(test.output, tokens) end
   for i, tok in ipairs(tokens) do
      if type(tok) ~= "table" then
         if tok ~= test.output[i] then
            check_fail(test.output, tokens)
         end
      else
         for j, attr in ipairs(tok) do
            if attr ~= test.output[i][j] then
               check_fail(test.output, tokens)
            end
         end
      end
   end
end

-- Don't mess up offsets when the input text starts with a BOM.
check{
   input = "\xef\xbb\xbffoo",
   output = {3},
   format = "offset",
}

-- Must not split tokens on extended diacritics.
check{
   input = "kr̥tya",
   output = {"kr̥tya"}
}

-- Segment correctly UTF-8 sequences, even when we don't have specific rules for
-- recognizing the underlying tokens.
check{
   input = "अभिनवगुप्त composed his विवरण before his तन्त्रालोक",
   format = {"str", "type"},
   output = {
      {"अभिनवगुप्त", "UNK"},
      {"composed", "LATIN"},
      {"his", "LATIN"},
      {"विवरण", "UNK"},
      {"before", "LATIN"},
      {"his", "LATIN"},
      {"तन्त्रालोक", "UNK"},
      }
}

-- Handle elisions correctly.
check{
   input = [[
      l'écart
      l'"écart"
      l’écart
      'l'
   ]],
   lang = "fr",
   format = {"str", "type"},
   output = {
      {"l'", "ELISION"}, {"écart", "LATIN"},
      {"l'", "ELISION"}, {'"', "SYM"}, {"écart", "LATIN"}, {'"', "SYM"},
      {"l’", "ELISION"}, {"écart", "LATIN"},
      {"'", "SYM"}, {"l", "LATIN"}, {"'", "SYM"},
   }
}

-- Handle suffixes correctly. We choose the longest possible suffix when there
-- are several possibilities.
check{
   input = [[
   dit-il
   a-t-il
   ]],
   lang = "fr",
   format = {"str", "type"},
   output = {
      {"dit", "LATIN"}, {"-il", "SUFFIX"},
      {"a", "LATIN"}, {"-t-il", "SUFFIX"},
   }
}

-- Token having both a prefix and a suffix.
check{
   input = "qu'a-t-il",
   lang = "fr",
   output = {"qu'", "a", "-t-il"},
}

-- Suffix splitting not applied to words in the langage lexicon.
check{
   input = "rendez-vous",
   lang = "fr",
   output = {"rendez-vous"}
}

-- Assimilations (only for English).
check{
   input = "'twas",
   lang = "en",
   output = {"'t", "was"}
}
check{
   input = "'twas",
   lang = "fr",
   output = {"'", "twas"}
}

-- Text chunks must be split on EM DASH, EN DASH, and --, at least when
-- tokenizing English text. But not if they look like years span.
check{
   input = "elements–such",
   output = {"elements", "–", "such"},
}
check{
   input = "elements—such",
   output = {"elements", "—", "such"},
}
check{
   input = "elements--such",
   output = {"elements", "--", "such"},
}
check{
   input = "1758–60",
   output = {"1758–60"},
}

-- Token with internal brackets.
check{
   input = [[
   (pré)opératoire désir(s) (MIP)-1alpha
   4-(parahydroxyphenyl)-2-butanone
   tris(dimethylamino)bromophosphonium
   d'écart(s)
   ]],
   output = {
   "(pré)opératoire", "désir(s)", "(MIP)-1alpha",
   "4-(parahydroxyphenyl)-2-butanone",
   "tris(dimethylamino)bromophosphonium",
   "d'", "écart(s)",
   },
   lang = "fr",
}

-- Path in the file system.
check{
   input = [[
      /usr/local/bin/qux
      /usr/foobar/
      ~/foo/bar
   ]],
   format = {"str", "type"},
   output = {
      {"/usr/local/bin/qux", "PATH"},
      {"/usr/foobar/", "PATH"},
      {"~/foo/bar", "PATH"},
   }
}

-- Numbers.
check{
   input = [[
      18 000 50 123 456
      9,435,000 square miles
      2,367.48 dollars
      2 367,48 euros
      3.45
      1290,3
      10e-4
      29
      +54
      0xdeadbeef
   ]],
   output = {
      "18 000 50 123 456",
      "9,435,000", "square", "miles",
      "2,367.48", "dollars",
      "2 367,48", "euros",
      "3.45",
      "1290,3",
      "10e-4",
      "29",
      "+54",
      "0xdeadbeef",
   }
}
-- Recognize hexadecimal as numbers.
check{
   input = "foo 0xdeadbeef bar",
   format = "type",
   output = {"LATIN", "NUM", "LATIN"},
}

-- Phone numbers, tokenized as a single unit.
check{
   input = "+1-541-754-3010 and (01) 55 1234 5678",
   output = {"+1-541-754-3010", "and", "(01) 55 1234 5678"},
}

-- Decades, abbreviated. We have a special rule for this to prevent 's from
-- being separated (which would happen otherwise).
check{
   input = "1920's 40's 80's",
   output = {
      "1920's", "40's", "80's",
   }
}

-- Phone number and email, in real context.
check{
   input = [[
   Salt Lake City, UT 84116, (801) 596-1887, email business@pglaf.org.
   ]],
   format = {"str", "type"},
   output = {
   {"Salt", "LATIN"},
   {"Lake", "LATIN"},
   {"City", "LATIN"},
   {",", "SYM"},
   {"UT", "LATIN"},
   {"84116", "NUM"},
   {",", "SYM"},
   {"(801) 596-1887", "NUM"},
   {",", "SYM"},
   {"email", "LATIN"},
   {"business@pglaf.org", "EMAIL"},
   {".", "SYM"},
   }
}
