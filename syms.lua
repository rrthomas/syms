#!/usr/bin/env lua
prog = {
  name = "syms",
  banner = "syms 0.86 (05 Oct 2012) by Reuben Thomas <rrt@sc3d.org>",
  purpose = "List symbols in input.",
  notes = "The default symbol type is words (-s \"([[:alpha:]]+)\"); other useful settings\n" ..
    "include:\n\n" ..
    "  non-white-space characters: -s \"[^[:space:]]+\"\n" ..
    "  alphanumerics and underscores: -s \"[[:alnum:]_]+\"\n" ..
    "  XML tags: -s \"<([a-zA-Z_:][a-zA-Z_:.0-9-]*)[[:space:]>]\""
}

require "std"
rex_posix = require "rex_posix"

-- Command-line options
options = {
  Option {{"symbol", "s"}, "symbols are given by REGEXP", "Req", "REGEXP"},
}

-- Parse command-line args
os.setlocale ("")
getopt.processArgs ()
local symbolPat = getopt.opt.symbol and table.remove (getopt.opt.symbol) or "([[:alpha:]]+)"

-- Compile symbol-matching regexp
local ok, pattern = pcall (rex_posix.new, symbolPat)
if not ok then
  die (pattern)
end

-- Process input
io.processFiles (function (file, number)
                   for line in io.lines () do
                     for s in rex_posix.gmatch (line, pattern) do
                       io.writelines (s)
                     end
                   end
               end)
