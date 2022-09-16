use std::fs::File;
use std::io::{self, BufRead};
use clap::{App, Arg, Error};
use regex::{Regex};

fn match_lines(pattern: &Regex, fp: impl BufRead)
{
    for line in fp.lines() {
        if let Ok(ip) = line {
            for mat in pattern.find_iter(&ip) {
                println!("{}", &ip[mat.start()..mat.end()]);
            }
        }
    }
}

fn main()
{
    // Command-line arguments
    let matches = App::new("syms")
        .version("0.11")
        .about("List symbols in input.")
        .author("Reuben Thomas")
        .arg(Arg::with_name("REGEXP")
             .short("s")
             .long("symbol")
             .takes_value(true)
             .default_value("\\p{L}+")
             .help("symbols are given by REGEXP"))
        .arg(Arg::with_name("FILE")
             .default_value("-")
             .multiple(true))
        .after_help("The default symbol type is words (-s \"\\p{L}+\"); other useful settings
include:

  non-white-space characters: -s \"\\S+\"
  alphanumerics and underscores: -s \"\\w+\"
  XML tags: -s \"<([a-zA-Z_:][a-zA-Z_:.0-9-]*)[\\s>]\"
")
        .get_matches();
    let symbol = matches.value_of("REGEXP").unwrap();

    // Compile symbol-matching regexp
    let pattern = match Regex::new(symbol) {
        Ok(regex) => regex,
        Err(error) => match error {
            regex::Error::Syntax(string) =>
                Error::exit(&Error {
                    message: string,
                    kind: clap::ErrorKind::InvalidValue,
                    info: None
                }),
            error => panic!("{}", error),
        },
    };

    // Process input
    for f in matches.values_of("FILE").unwrap() {
        if f == "-" {
            match_lines(&pattern, io::stdin().lock());
        } else {
            let fp = match File::open(f) {
                Ok(file) => io::BufReader::new(file),
                Err(e) => Error::exit(&Error {
                    message: e.to_string(),
                    kind: clap::ErrorKind::Io,
                    info: None
                }),
            };
            match_lines(&pattern, fp);
        }
    }
}
