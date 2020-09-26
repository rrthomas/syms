#!/usr/bin/env python3

import os
import sys
import argparse
import locale
import re
import fileinput

# Command-line arguments
parser = argparse.ArgumentParser(description='List symbols in input.',
                                 epilog='''
The default symbol type is words (-s "([^\W\d_]+)"); other useful settings
include:

  non-white-space characters: -s "(\S+)"
  alphanumerics and underscores: -s "(\w+)"
  XML tags: -s "<([a-zA-Z_:][a-zA-Z_:.0-9-]*)[\s>]"
''',
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
parser.add_argument('-s', '--symbol', metavar='REGEXP', type=os.fsencode, default=b'[^\W\d_]+',
                    help='symbols are given by REGEXP')
parser.add_argument('-V', '--version', action='version',
                    version='%(prog)s 0.9 (14 Jan 2012) by Reuben Thomas <rrt@sc3d.org>')
parser.add_argument('file', metavar='FILE', nargs='*')

args = parser.parse_args()

# Set locale
locale.setlocale(locale.LC_ALL, '')

# Compile symbol-matching regexp
try:
    pattern = re.compile(args.symbol, re.LOCALE)
except re.error as err:
    parser.error(err.args[0])

# Process input
for f in args.file or ['-']:
    for line in fileinput.input(mode='rb', files=(f,)):
        for s in pattern.findall(line):
            sys.stdout.buffer.write(s + b"\n")
