#!/usr/bin/env ruby

# By Reuben Thomas, based on Wybo Dekker's "words"

require 'optparse'

NAME = "occurs"

# Command-line arguments
pat, nocount = Regexp.new('\w+'), false
ARGV.options do |opt|
  opt.banner = "#{NAME} - print, and optionally count, words occurring in a list of files"
  opt.on("-n", "--nocount",
         "don't show the frequencies or total") do
    nocount = true end
  opt.on("-s", "--symbol=REGXEP",
         "symbols are given by REGEXP") do |v|
    pat = Regexp.new(v)
  end
  opt.on("-V", "--version",
         "show script version and exit") do
    puts "#{NAME} 0.9 (27 Sep 2011)"
    exit
  end
  opt.parse!
end

# Read input
words = {}
while gets do
  scan(pat).each do |k|
    words[k] ||= 0
    words[k] += 1
  end
end

# Output results
warn "#{words.length} symbols" unless nocount
words.each do |k,v|
  puts nocount ? k : "#{v} #{k}"
end
