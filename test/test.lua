local json = require("json")
local mascara = require("mascara")

local all_good = true

local function check_fail(expected, output)
   local caller_caller = assert(debug.getinfo(3))
   print(string.format("fail at line %d:", caller_caller.currentline))
   print("== Expected:")
   print(json.stringify(expected))
   print("== Have:")
   print(json.stringify(output))
   print("output len = ", #output[1])
   all_good = false
end

local function add_token(tokens, format, tk)
   if #format == 1 then
      local attr = format[1]
      table.insert(tokens, assert(tk[attr]))
   else
      local tok = {}
      for _, attr in ipairs(format) do
         table.insert(tok, assert(tk[attr]))
      end
      table.insert(tokens, tok)
   end
end

local function check_token(test)
   local tokens = {}
   local format = test.format or {"str"}
   if type(format) == "string" then
      format = {format}
   end
   local mr = mascara.new(test.lang or "en")
   mr:set_text(test.input)
   while true do
      local tk = mr:next()
      if not tk then break end
      add_token(tokens, format, tk)
   end
   if #tokens ~= #test.output then return check_fail(test.output, tokens) end
   for i, tk in ipairs(tokens) do
      if type(tk) ~= "table" then
         if tk ~= test.output[i] then
            return check_fail(test.output, tokens)
         end
      else
         for j, attr in ipairs(tk) do
            if attr ~= test.output[i][j] then
               return check_fail(test.output, tokens)
            end
         end
      end
   end
end

local sentencizers = setmetatable({}, {__index = function(t, lang)
   local szr = rawget(t, lang)
   if not szr then
      szr = mascara.new(lang, "sentence")
      t[lang] = szr
   end
   return szr
end})

local function check_sentence_impl(config, test)
   local sents = {}
   local szr = sentencizers[config]
   szr:set_text(test.input)
   while true do
      local sent = szr:next()
      if not sent then break end
      local copy = {}
      for _, tk in ipairs(sent) do
         table.insert(copy, tk.str)
      end
      table.insert(sents, copy)
   end
   if #sents ~= #test.output then return check_fail(test.output, sents) end
   for i, sent in ipairs(sents) do
      if #sent ~= #test.output[i] then
         return check_fail(test.output, sents)
      end
      for j, tk in ipairs(sent) do
         if tk ~= test.output[i][j] then
            return check_fail(test.output, sents)
         end
      end
   end
end

local function check_sentence(test)
   local lang = test.lang or "en"
   local impls = test.impl and {test.impl} or {"fsm", "bayes"}
   for _, impl in ipairs(impls) do
      check_sentence_impl(lang .. " " .. impl, test)
   end
end

check = check_token
dofile("token.lua")

check = check_sentence
dofile("sentence.lua")

if not all_good then error("fail!") end
