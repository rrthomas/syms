///usr/bin/env go run $0 $@ ; exit

package main

import (
	"bufio"
	"flag" // FIXME: use getopt or go-flags
	"fmt"
	"os"
	"log"
	"regexp"
	"github.com/famz/SetLocale"
)

var progname = "syms"
var version = "0.92 (08 Dec 2014)"
var author = "Reuben Thomas <rrt@sc3d.org>"

// Command-line arguments
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
		"  XML tags: -s \"<([a-zA-Z_:][a-zA-Z_:.0-9-]*)[\\s>]\"\n")
}

func showVersion() {
	os.Stderr.WriteString(progname + " " + version + " " + author + "\n")
}

func main() {
	SetLocale.SetLocale(SetLocale.LC_ALL, "")

	// Parse command-line args
	flag.Parse()
	if *versionFlag {
		showVersion()
	} else if *helpFlag {
		usage()
	} else {
		// Compile symbol-matching regexp
		pattern, err := regexp.Compile(*symbol)
		if err != nil {
			panic(err)
		}

		// Process input
		args := flag.Args()
		if flag.NArg() == 0 {
			args = append(args, "-")
		}
		for i := range args {
			h := os.Stdin
			f := args[i]
			if f != "-" {
				h, err = os.Open(f)
				if err != nil {
					log.Fatal(err)
				}
			}

			// Read file into symbol table
			scanner := bufio.NewScanner(h)
			for scanner.Scan() {
			line := scanner.Text()
				if scanner.Err() != nil {
					log.Fatal(err)
				}
				for _, matches := range pattern.FindAllStringSubmatch(line, -1) {
					fmt.Println(string(matches[1]))
				}
			}
			h.Close()
		}
	}
}
