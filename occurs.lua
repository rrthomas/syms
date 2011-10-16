#!/usr/bin/env lua
prog = {
  name = "occurs",
  banner = "occurs 0.85 (27 Sep 2011) by Reuben Thomas <rrt@sc3d.org>",
  purpose = "Count the occurrences of each symbol in a file.",
  notes = "The default symbol type is words (-s \"([[:alpha:]]+)\"); other useful settings\n" ..
    "include:\n\n" ..
    "  non-white-space characters: -s \"[^[:space:]]+\"\n" ..
    "  alphanumerics and underscores: -s \"[[:alnum:]_]+\"\n" ..
    "  XML tags: -s \"<([a-zA-Z_:][a-zA-Z_:.0-9-]*)[[:space:]>]\""
}

require "std"
require "rex_posix"

-- Command-line options
options = {
  Option {{"nocount", "n"}, "don't show the frequencies or total"},
  Option {{"symbol", "s"}, "symbols are given by REGEXP", "Req", "REGEXP"},
}

-- Parse command-line args
os.setlocale ("")
getopt.processArgs ()
local symbolPat = getopt.opt.symbol or "([[:alpha:]]+)"

-- Compile symbol-matching regexp
local ok, pattern = pcall (rex_posix.new, symbolPat)
if not ok then
  die (pattern)
end

-- Process input
local freq = {}
io.processFiles (function (file, number)
                   for line in io.lines () do
                     for s in rex_posix.gmatch (line, pattern) do
                       freq[s] = (freq[s] or 0) + 1
                     end
                   end
               end)

-- Write output
local symbols = 0
for s in pairs (freq) do
  io.writeline (s .. (getopt.opt.nocount and "" or " " .. freq[s]))
  symbols = symbols + 1
end
if not getopt.opt.nocount then
  warn ("total symbols: " .. symbols)
end
