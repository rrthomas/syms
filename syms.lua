#!/usr/bin/env lua
spec = [=[
syms 0.91
Copyright (c) 2022 Reuben Thomas <rrt@sc3d.org>

Usage: syms [OPTION...] [FILE...]

List symbols in input.

The default symbol type is words (-s "[[:alpha:]]+"); other useful settings
include:

  non-white-space characters: -s "[^[:space:]]+"
  alphanumerics and underscores: -s "[[:alnum:]_]+"
  XML tags: -s "<([a-zA-Z_:][a-zA-Z_:.0-9-]*)[[:space:]>]"

Options:

  -s, --symbol=REGEXP      symbols are given by REGEXP
      --help               display this help, then exit
      --version            display version information, then exit
]=]

local std = require "std"
rex_posix = require "rex_posix"

-- Parse command-line args
os.setlocale ("")
local OptionParser = require "std.optparse"
local parser = OptionParser (spec)
_G.arg, opts = parser:parse (_G.arg)
local symbolPat = opts.symbol or "[[:alpha:]]+"

-- Compile symbol-matching regexp
local ok, pattern = pcall (rex_posix.new, symbolPat)
if not ok then
  std.io.die (pattern)
end

-- Process input
std.io.process_files (function (file, number)
                        -- FIXME: make slurp work in pipes
                        for s in rex_posix.gmatch (std.io.slurp (), pattern) do
                          std.io.writelines (s)
                        end
                      end)
