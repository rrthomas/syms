# syms

`syms` lists symbols in its input (where "symbol" can be defined flexibly enough to be words, identifiers or XML tags).

syms is distributed under the terms of the GNU General Public License; either
version 3 of the License, or (at your option), any later version. See the
file COPYING for more details.

## Usage examples

Unique symbols, sorted lexically: `syms "$@" | sort | uniq`  
Number of unique symbols: `syms "$@" | sort | uniq | wc -l`  
Unique symbols, sorted by frequency: `syms "$@" | sort | uniq -c | sort -n -k 2`  
Unique lines (like uniq, but not just adjacent lines): `syms -s "^(.*)$" "$@"`  

```
usage: syms [-h] [-s REGEXP] [-V] [FILE ...]

List symbols in input.

positional arguments:
  FILE

options:
  -h, --help            show this help message and exit
  -s REGEXP, --symbol REGEXP
                        symbols are given by REGEXP
  -V, --version         show program's version number and exit

The default symbol type is words (-s "\p{L}+"); other useful settings
include:

  non-white-space characters: -s "\S+"
  alphanumerics and underscores: -s "\w+"
  XML tags: -s "<([a-zA-Z_:][a-zA-Z_:.0-9-]*)[\s>]"
```
