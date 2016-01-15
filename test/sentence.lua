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
