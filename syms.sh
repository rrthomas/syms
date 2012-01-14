#!/bin/sh
# syms
# List symbols in input
# Reuben Thomas   18/2/98, 14/1/12

# Suggestion from Underhanded C contest 2006:
#tr "[:space:]" "[\n*]" | sort | awk 'length($0)>0' | uniq -c
tr -cs A-Za-z '\n' < $1 | grep [[:print:]]
