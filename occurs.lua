#!/usr/bin/env lua
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
