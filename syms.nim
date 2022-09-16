import os
import system
import argparse
import re
import logging
import strformat


const progName = "syms" # FIXME: allow newParser to take a variable, then use: lastPathPart(getAppFilename())

# Error messages
var logger = newConsoleLogger(fmtStr = &"{progName}: ")
addHandler(logger)
proc die(s: string) =
  warn(s)
  quit(1)

# Command-line arguments
var p = newParser(progName):
  help("List symbols in input.")
  option("-s", "--symbol", default = "\\p{L}+",
         help = "symbols are given by the regexp SYMBOL")
  flag("-V", "--version",
       help = "show program's version number and exit")
  arg("file", nargs = -1)
  nohelpflag()
  flag("-h", "--help", help = "show this help") # FIXME: downcase help message for --help

try:
  let opts = p.parse()
  if opts.help:
    echo """

The default symbol type is words (-s "\p{L}+"); other useful settings
include:

  non-white-space characters: -s "\S+"
  alphanumerics and underscores: -s "\w+"
  XML tags: -s "<([a-zA-Z_:][a-zA-Z_:.0-9-]*)[\s>]""""
    quit(0)
  if opts.version:
    echo &"{progName} 0.12 (16 Sep 2022) by Reuben Thomas <rrt@sc3d.org>"
    quit(0)

  # Compile symbol-matching regexp
  let pattern = re(opts.symbol)

  # Process input
  for f in opts.file #[or ['-']]#:
    for line in f.lines:
      for s in findAll(line, pattern):
        echo s

except: die(getCurrentExceptionMsg())
