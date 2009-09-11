#!/bin/sh
# occurs
# Count number of occurrences of words in a file
# Reuben Thomas   18/2/98

# Suggestion from Underhanded C contest 2006:
#tr "[:space:]" "[\n*]" | sort | awk 'length($0)>0' | uniq -c
tr A-Z a-z < $1 | tr -cs A-Za-z '\n' | grep [[:print:]] | sort | uniq -c | sort
