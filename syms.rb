#!/usr/bin/env ruby

# By Reuben Thomas, based on Wybo Dekker's "words"

require 'optparse'

NAME = "syms"

# Command-line arguments
pat, nocount = Regexp.new('\w+'), false
ARGV.options do |opt|
  opt.banner = "#{NAME} - list symbols in input"
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

# Process input
ARGF.each do |s|
  s.scan(pat).each do |k|
    puts k
  end
end
