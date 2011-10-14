#!/usr/bin/env python3

import sys
import argparse
import locale
import re
import fileinput
from collections import Counter

# Command-line arguments
parser = argparse.ArgumentParser(prog='occurs',
                                 description='Count the occurrences of each symbol in a file.',
                                 epilog='''
The default symbol type is words (-s "([^\W\d_]+)"); other useful settings
include:

  non-white-space characters: -s "(\S+)"
  alphanumerics and underscores: -s "(\w+)"
  XML tags: -s "<([a-zA-Z_:][a-zA-Z_:.0-9-]*)[\s>]"
''',
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
parser.add_argument('-n', '--nocount', action='store_true',
                    help='don\'t show the frequencies or total')
parser.add_argument('-s', '--symbol', metavar='REGEXP', default='[^\W\d_]+',
                    help='symbols are given by REGEXP')
parser.add_argument('-V', '--version', action='version',
                    version='%(prog)s 0.9 (27 Sep 2011) by Reuben Thomas <rrt@sc3d.org>')
parser.add_argument('file', metavar='FILE', nargs='*')

args = parser.parse_args()

# Set locale
locale.setlocale(locale.LC_ALL, '')

# Compile symbol-matching regexp
try:
    pattern = re.compile(args.symbol, re.LOCALE)
except re.error as err:
    parser.error(err.args[0])

# Process a file
def occurs(h, f):
    freq = Counter()
    for line in h:
        freq.update(pattern.findall(line))
    if not args.nocount:
        print("{}: {} symbols".format(f, len(freq)), file=sys.stderr)
    for s in freq:
        print(s, end='')
        if not args.nocount:
            print(' {}'.format(freq[s]), end='')
        print('')


args.file = args.file or ['-']
for i, f in enumerate(args.file):
    if i > 0:
        print('')
    h = fileinput.input(files=(f,))
    occurs(h, f)
    h.close()
