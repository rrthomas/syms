#! /usr/bin/env lua
prog = {
  name = "occurs",
  banner = "occurs 0.83 (25 Sep 2008) by Reuben Thomas <rrt@sc3d.org>",
  purpose = "Count the occurrences of each symbol in a file",
  notes = "The default symbol type is words (-s \"%a+\"); other " ..
    "useful settings include:\n\n" ..
    "  non-white-space characters: -s \"%S+\"\n" ..
    "  alphanumerics and underscores: -s \"[%w_]+\"\n" ..
    "  SGML tags: -s \"[a-zA-Z]\" -l \"\<\" -r \"[%s\\>]\""
}

-- FIXME: rewrite in Python
-- FIXME: bullet-proof by writing checkRegex

require "std"


-- Process a file
function occurs (_, number)
  local symbol, freq = {}, {}
  for line in io.lines () do
    string.gsub (line, leftDel .. "(" .. symbolPat .. ")" .. rightDel,
                 function (s)
                   if getopt.opt.lower then
                     s = string.lower (s)
                   end
                   if not freq[s] then
                     table.insert (symbol, s)
                     freq[s] = 1
                   else
                     freq[s] = freq[s] + 1
                   end
                 end)
  end
  if getopt.opt.sort == "lexical" then
    table.sort (symbol)
  elseif getopt.opt.sort == "frequency" then
    table.sort (symbol,
                function (a, b)
                  return freq[a] > freq[b]
                end)
  end
  if not getopt.opt.nocount then
    io.stderr:write (tostring (#symbol) .. " symbols\n")
  end
  for s in list.elems (symbol) do
    io.write (s)
    if not getopt.opt.nocount then
      io.write (" " .. tostring (freq[s]))
    end
    io.write ("\n")
  end
  if number < #arg then
    io.write ("\n")
  end
end

-- Command-line options
options = {
  Option {{"sort", "S"},
    "sort by given method (one of `lexical', `frequency')",
    "Req", "METHOD"},
  Option {{"nocount", "n"}, "don't show the frequencies or total"},
  Option {{"symbol", "s"}, "symbols are given by REGEXP",
    "Req", "REGEXP"},
  Option {{"left", "l"}, "symbols must be preceded by REGEXP",
    "Req", "REGEXP"},
  Option {{"right", "r"}, "symbols must be followed by REGEXP",
    "Req", "REGEXP"},
  Option {{"lower", "L"}, "symbols are converted to lower case"},
}

-- Main routine
os.setlocale ("")
getopt.processArgs ()
symbolPat, leftDel, rightDel = "%a+", "", ""
if getopt.opt.symbol then
  symbolPat = getopt.opt.symbol
end
if getopt.opt.left then
  leftDel = getopt.opt.left
end
if getopt.opt.right  then
  rightDel = getopt.opt.right
end
-- FIXME: checkRegex (symbolPat, leftDel, rightDel)
io.processFiles (occurs)


-- Changelog
-- 25sep08  0.83 Use io.lines and list.elems iterators
-- 16nov07  0.82 Change one last table.n to #table
-- 24dec06  0.81 Fix remaining space before argument in documentation
-- 20oct06  0.8  Fix documentation: command-line arguments always need
--               space before; change "word" to "symbol"
--               Change -byfreqs to -sort to allow no sorting; make
--               that the default
--               Change -nofreqs to -nocount, and if used, don't
--               display total number of symbols
--               Anonymised one-use local functions
-- 08oct06  0.75 table.getn --> #
-- 29mar06  0.74 Updated to match stdlib
-- 05may05  0.73 Updated to match stdlib
-- 15jan05  0.59 (Updated C version)
-- 29jan04  0.72 Don't die with usage if no arguments given
-- 26jan04  0.71 Updated to match stdlib
-- 21oct03  0.70 Updated to match stdlib
--               Don't die with usage when some options are given but
--               no files
-- 18oct03  0.69 Updated to match stdlib
-- 12sep03  0.68 Update for Lua 5
-- 12aug02  0.67 Now need to print usage message explicitly if no args
-- 22jun02  0.66 Updated to match stdlib
-- 18jan02  0.65 Code rearranged
-- 03jan02  0.64 Updated to match stdlib
-- 11aug01  0.63
-- 27jul01  0.62
-- 17jul01  0.61
-- 02jul01  0.6  Lua version
-- 30jun01- 0.58 Eric Hutton's (bookman@rmplc.co.uk) comments on the
-- 25nov97  0.4  BASIC version caused me to translate it to C and
--               improve it.
-- 23nov97- 0.33 BBC BASIC V version
-- 25jan92  0.1
