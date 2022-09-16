#!/usr/bin/env python3

import os
import argparse
import locale
import regex
import fileinput

# Command-line arguments
parser = argparse.ArgumentParser(description='List symbols in input.',
                                 epilog='''
The default symbol type is words (-s "\p{L}+"); other useful settings
include:

  non-white-space characters: -s "\S+"
  alphanumerics and underscores: -s "\w+"
  XML tags: -s "<([a-zA-Z_:][a-zA-Z_:.0-9-]*)[\s>]"
''',
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
parser.add_argument('-s', '--symbol', metavar='REGEXP', default=r'\p{L}+',
                    help='symbols are given by REGEXP')
parser.add_argument('-V', '--version', action='version',
                    version='%(prog)s 0.91 (16 Sep 2022) by Reuben Thomas <rrt@sc3d.org>')
parser.add_argument('file', metavar='FILE', nargs='*')

args = parser.parse_args()

# Set locale
locale.setlocale(locale.LC_ALL, '')

# Compile symbol-matching regexp
try:
    pattern = regex.compile(args.symbol)
except regex.error as err:
    parser.error(err.args[0])

# Process input
for line in fileinput.input(files=args.file or ['-']):
    for s in pattern.findall(line):
        print(s)
