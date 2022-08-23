import os
import system
import argparse
#import posix
import re
import logging
import strformat


let progName = lastPathPart(getAppFilename())

# Error messages
var logger = newConsoleLogger(fmtStr = &"{progName}: ")
addHandler(logger)
proc die(s: string) =
  warn(s)
  quit(1)

# Command-line arguments
var p = newParser("syms"):
  help("List symbols in input.")
  option("-s", "--symbol", default = "[^\\W\\d_]+",
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

The default symbol type is words (-s "([^\W\d_]+)"); other useful settings
include:

  non-white-space characters: -s "(\S+)"
  alphanumerics and underscores: -s "(\w+)"
  XML tags: -s "<([a-zA-Z_:][a-zA-Z_:.0-9-]*)[\s>]""""
    quit(0)
  if opts.version:
    echo &"{progName} 0.1 (26 Sep 2020) by Reuben Thomas <rrt@sc3d.org>"
    quit(0)

  # Set locale
  #discard setlocale(LC_ALL, "")

  # Compile symbol-matching regexp
  # FIXME: use locale-sensitive regexs
  let pattern = re(opts.symbol)

  # Process input
  for f in opts.file #[or ['-']]#:
    for line in f.lines:
      for s in findAll(line, pattern):
        echo s

except: die(getCurrentExceptionMsg())
