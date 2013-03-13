#!/bin/sh
# syms
# List symbols in input
# Reuben Thomas   18/2/98, 14/1/12, 13/3/13

# Until tr is fixed to work with multibyte encodings, this won't work properly
tr -cs [:alpha:] '\n' < $1 | grep [[:print:]]
