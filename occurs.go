#!/usr/bin/env gorun

package main

import (
	"io"
	"bufio"
	"os"
	"fmt"
	"flag"
	"regexp"
	"strings"
)

var progname = "occurs"
var version = "0.9 (27 Sep 2011)"
var author = "Reuben Thomas <rrt@sc3d.org>"

// Command-line arguments
var nocount *bool = flag.Bool("nocount", false, "don't show the frequencies or total")
var symbol *string = flag.String("symbol", "[A-Za-z]+", "symbols are given by REGEXP")
var left *string = flag.String("left", "", "symbols must be preceded by REGEXP")
var right *string = flag.String("right", "", "symbols must be followed by REGEXP")
var downcase *bool = flag.Bool("lower", false, "symbols are converted to lower case")
var versionFlag *bool = flag.Bool("version", false, "output version information and exit")
var helpFlag *bool = flag.Bool("help", false, "display this help and exit")

func usage() {
	os.Stderr.WriteString(progname + " " + version + "\n\n" +
		"Count the occurrences of each symbol in a file.\n\n")
	flag.Usage()
	os.Stderr.WriteString("\n" +
		"The default symbol type is words (-s \"[^\\W\\d_]+\"); other useful settings\n" +
		"include:\n" +
		"\n" +
		"  non-white-space characters: -s \"[ \\t\\n\\f\\v]+\"\n" +
		"  alphanumerics and underscores: -s \"[A-Za-z0-9_]+\"\n" +
		"  XML tags: -s \"[a-zA-Z_:][a-zA-Z_:.0-9-]*\" -l \"<\" -r \"[\\s>]\"\n")
}

func showVersion() {
	os.Stderr.WriteString(progname + " " + version + " " + author + "\n")
}

type FreqSlice struct {
	symbol []string
	freq map[string]int
}

func die(msg string) {
	os.Stderr.WriteString(progname + ": " + msg)
	os.Exit(1)
}

func dieWithError(err os.Error) {
	die(err.String() + "\n")
}

// Process a file
func occurs(h io.Reader, f string, pattern *regexp.Regexp) {
	// Read file into symbol table
	bh := bufio.NewReader(h)
	sl := &FreqSlice{ make([]string, 0, 0), make(map[string]int) }
	for {
		line, err := bh.ReadString('\n')
		if err == os.EOF { break }
		syms := pattern.FindAllStringSubmatch(line, -1)
		for _, matches := range syms {
			s := matches[1]
			if *downcase { s = strings.ToLower(s) }
			if sl.freq[s] == 0 { sl.symbol = append(sl.symbol, s) }
			sl.freq[s] += 1
		}
	}

	// Print out symbol data
	if !*nocount { fmt.Fprintf(os.Stderr, "%s: %d symbols\n", f, len(sl.symbol)) }
	for _, s := range sl.symbol {
		fmt.Print(s)
		if !*nocount { fmt.Printf(" %d", sl.freq[s]) }
		fmt.Print("\n")
	}
}

func main() {
	// FIXME: setlocale(locale.LC_ALL, '')

	// Parse command-line args
	flag.Parse()
	if *versionFlag { showVersion(); os.Exit(0) }
	if *helpFlag { usage(); os.Exit(0) }

	// Compile symbol-matching regexp
	pattern, err := regexp.Compile(*left + "(" + *symbol + ")" + *right)
	if err != nil { dieWithError(err) }
	pattern.MatchString("foo")

	// Process input
	if flag.NArg() == 0 {
		occurs(os.Stdin, "-", pattern)
	} else {
		for i := 0; i < flag.NArg(); i++ {
			var h io.Reader;
			f := flag.Arg(i)
			if f != "-" {
				var err os.Error
				h, err = os.Open(f)
				if err != nil { dieWithError(err) }
			} else { h = os.Stdin }
			occurs(h, f, pattern)
			if i < flag.NArg() - 1 { os.Stdout.WriteString("\n") }
		}
	}
}
