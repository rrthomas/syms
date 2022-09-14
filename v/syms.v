module main

import os
import flag
import v.vmod

const manifest = vmod.from_file('v.mod') or { panic(err) }

#include "regex.h"

const reg_extended = 1

[typedef]
struct C.regex_t {}

[typedef]
struct C.regmatch_t {
	rm_so isize
	rm_eo isize
}

fn C.regcomp(&C.regex_t, &char, int) int
fn C.regexec(&C.regex_t, &char, usize, []C.regmatch_t, int) int
fn C.regerror(int, &C.regex_t, &char, usize) usize

[noreturn]
fn error_exit(code int, msg string) {
	eprintln(msg)
	exit(code)
}

fn get_symbol(re &C.regex_t, s string, start isize) (isize, isize) {
	matches := []C.regmatch_t{len: 2}
	if C.regexec(re, unsafe { s.str + start }, 1, matches.data, 0) != 0 {
		return -1, -1
	}
	return start + matches[0].rm_so, start + matches[0].rm_eo
}

fn main() {
	mut fp := flag.new_flag_parser(os.args)
	fp.application(manifest.name)
	fp.version(manifest.version)
	fp.description(manifest.description)
	default_symbol := '[[:alpha:]]+'
	symbol := fp.string('symbol', `s`, default_symbol, 'symbols are given by REGEXP')
	fp.footer('
The default symbol type is words (-s "$default_symbol"); other useful settings include:

  non-white-space characters: -s "[^[:space:]]+"
  alphanumerics and underscores: -s "[[:alnum:]_]+"
  XML tags: -s "<([a-zA-Z_:][a-zA-Z:.0-9-]*)[[:space:]>]"')
	fp.skip_executable()

	// Compile regex
	re := C.regex_t{}
	err := C.regcomp(&re, symbol.str, reg_extended)
	if err != 0 {
		errlen := C.regerror(err, &re, &char(0), 0)
		errbuf := []char{len: int(errlen)}
		C.regerror(err, &re, &errbuf[0], errlen)
		error_exit(1, unsafe { (&errbuf[0]).vstring() })
	}

	additional_args := fp.finalize()?
	for file in additional_args {
		mut stdin := os.stdin()
		stdin.reopen(file, 'r') or { error_exit(1, 'cannot open \'$file\'') }
		for !stdin.eof() {
			l := os.get_line()
			for start, end := isize(0), isize(0); true; start = end {
				start, end = get_symbol(&re, l, start)
				if start == -1 {
					break
				}
				println('${l[start..end]}')
			}
		}
	}
}
