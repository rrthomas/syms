#!/usr/bin/env lua
spec = [=[
syms 0.88
Copyright (c) 2014 Reuben Thomas <rrt@sc3d.org>

Usage: syms [OPTION...] [FILE...]

List symbols in input.

The default symbol type is words (-s "([[:alpha:]]+)"); other useful settings
include:

  non-white-space characters: -s "[^[:space:]]+"
  alphanumerics and underscores: -s "[[:alnum:]_]+"
  XML tags: -s "<([a-zA-Z_:][a-zA-Z_:.0-9-]*)[[:space:]>]"

Options:

  -s, --symbol=REGEXP      symbols are given by REGEXP
  -h, --help               display this help, then exit
      --version            display version information, then exit
]=]

require "std"
rex_posix = require "rex_posix"


-- Parse command-line args
os.setlocale ("")
local OptionParser = require "std.optparse"
local parser = OptionParser (spec)
_G.arg, opts = parser:parse (_G.arg)
local symbolPat = opts.symbol or "([[:alpha:]]+)"

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
