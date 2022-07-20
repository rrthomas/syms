module main

import os
import flag
import regex // FIXME: use POSIX regexs

import v.vmod
const manifest = vmod.from_file('v.mod') or { panic(err) }

[noreturn]
fn error_exit(code int, msg string) {
	eprintln(msg)
	exit(code)
}

fn main() {
	mut fp := flag.new_flag_parser(os.args)
	fp.application(manifest.name)
	fp.version(manifest.version)
	fp.description(manifest.description)
	default_symbol := '[\\a\\A]+'
	symbol := fp.string('symbol', `s`, default_symbol, 'symbols are given by REGEXP')
	mut re := regex.regex_opt(symbol) or {
		error_exit(1, 'invalid regex $symbol')
	}
	fp.footer('
The default symbol type is words (-s "$default_symbol"); other useful settings include:

  non-white-space characters: -s "[\\S]+"
  alphanumerics and underscores: -s "\\w+"
  XML tags: -s "<([\\a\\a_:][\\w:.-]*)[\\s>]"')
	fp.skip_executable()

	additional_args := fp.finalize() ?
	for file in additional_args {
		// FIXME: read one line at a time
		lines := os.read_lines(file) or {
			error_exit(1, 'cannot open \'$file\'')
		}
		for l in lines {
			// FIXME: read one symbol at a time
			for sym in re.find_all_str(l) {
				println(sym)
			}
		}
	}
}
