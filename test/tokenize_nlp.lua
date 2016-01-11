-- Overlapping prefixes and suffixes. In this case, we emit the prefix and treat
-- the rest of the string as a single token. This is an arbitrary choice.
check{
   -- We add "C'est" and "he's" to make sure that "C'" and "'s" are recognized;
   -- otherwise we wouldn't test anything.
   input = "C'est he's C's",
   output = {
      "C'", "est", "he", "'s", "C'", "s", 
   }
}

-- Ensure that we recognize paragraph breaks when appropriate.
check{
   input = "foo\nbar",
   output = {"LATIN", "LATIN"},
   format = "type",
}
check{
   input = "foo\n\nbar",
   output = {"LATIN", "PARA", "LATIN"},
   format = "type",
}
check{
   input = "foo\r\nbar",
   output = {"LATIN", "LATIN"},
   format = "type",
}
check{
   input = "foo\r\n\nbar",
   output = {"LATIN", "PARA", "LATIN"},
   format = "type",
}
