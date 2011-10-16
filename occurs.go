#!/usr/bin/env gorun

package main

import (
	"bufio"
	"os"
	"fmt"
	"flag"
	"regexp"
)

var progname = "occurs"
var version = "0.9 (27 Sep 2011)"
var author = "Reuben Thomas <rrt@sc3d.org>"

// Command-line arguments
var nocount *bool = flag.Bool("nocount", false, "don't show the frequencies or total")
var symbol *string = flag.String("symbol", "([A-Za-z]+)", "symbols are given by REGEXP")
var versionFlag *bool = flag.Bool("version", false, "output version information and exit")
var helpFlag *bool = flag.Bool("help", false, "display this help and exit")

func usage() {
	os.Stderr.WriteString(progname + " " + version + "\n\n" +
		"Count the occurrences of each symbol in a file.\n\n")
	flag.Usage()
	os.Stderr.WriteString("\n" +
		"The default symbol type is words (-s \"([^\\W\\d_]+)\"); other useful settings\n" +
		"include:\n" +
		"\n" +
		"  non-white-space characters: -s \"([ \\t\\n\\f\\v]+)\"\n" +
		"  alphanumerics and underscores: -s \"([A-Za-z0-9_]+)\"\n" +
		"  XML tags: -s \"<([a-zA-Z_:][a-zA-Z_:.0-9-]*[\\s>]\"\n")
}

func showVersion() {
	os.Stderr.WriteString(progname + " " + version + " " + author + "\n")
}

func readline (h *bufio.Reader) ([]byte, os.Error) {
	var line []byte;
	for l, isPrefix, err := h.ReadLine(); true; l, isPrefix, err = h.ReadLine() {
		line = append(line, l...)
		if err != nil || !isPrefix { return line, err }
	}
	return line, nil
}

func main() {
	defer func () {
		if r := recover(); r != nil {
			fmt.Fprintf(os.Stderr, "%s: %s\n", progname, r)
			os.Exit(1)
		}
	}()

	// FIXME: setlocale(locale.LC_ALL, '')

	// Parse command-line args
	flag.Parse()
	if *versionFlag { showVersion(); os.Exit(0) }
	if *helpFlag { usage(); os.Exit(0) }

	// Compile symbol-matching regexp
	pattern, err := regexp.Compile(*symbol)
	if err != nil { panic(err) }

	// Process input
	symbol := make([]string, 0, 0)
	freq := make(map[string]int)
	args := flag.Args()
	if flag.NArg() == 0 { args = append(args, "-") }
	for i := range args {
		var h *os.File;
		f := args[i]
		if f != "-" {
			var err os.Error
			h, err = os.Open(f)
			if err != nil { panic(err) }
			defer h.Close()
		} else { h = os.Stdin }

		// Read file into symbol table
		bh := bufio.NewReader(h)
	read:
		for {
			line, err := readline(bh)
			switch err {
			case nil:
				syms := pattern.FindAllSubmatch(line, -1)
				for _, matches := range syms {
					s := string(matches[1])
					if freq[s] == 0 { symbol = append(symbol, s) }
					freq[s] += 1
				}
			case os.EOF:
				break read
			default:
				panic(err)
			}
		}
	}

	// Print out symbol data
	for _, s := range symbol {
		fmt.Print(s)
		if !*nocount { fmt.Printf(" %d", freq[s]) }
		fmt.Print("\n")
	}
	if !*nocount { fmt.Fprintf(os.Stderr, "Total symbols: %d\n", len(symbol)) }
}
