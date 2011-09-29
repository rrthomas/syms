// occurs
// Counts the number of occurrences of each symbol in a text file
// Reuben Thomas (rrt@sc3d.org)


// FIXME: Cope with wide character encodings.

#include <config.h>

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <getopt.h>
#include <regex.h>
#include "xalloc.h"
#include "hash.h"

#include "cmdline.h"


struct gengetopt_args_info args_info;

// Type to hold symbol-frequency pairs
typedef struct freq_symbol {
  char *symbol;
  size_t count;
} *freq_symbol_t;

static size_t
symbolhash(const void *v, size_t n)
{
  return hash_string(((const struct freq_symbol *) v)->symbol, n);
}

static bool
symboleq(const void *v, const void *w)
{
  return strcmp(((const struct freq_symbol *) v)->symbol, ((const struct freq_symbol *) w)->symbol) == 0;
}

static regex_t symbol_re; // regex object for the symbol pattern

static char *
get_symbol(char *s, char **end)
{
  regmatch_t match[1];
  if (regexec(&symbol_re, s, 1, match, 0) != 0)
    return NULL;
  *end = s + match[0].rm_eo;
  return s + match[0].rm_so;
}

// Read the file into a hash
static size_t
read_symbols(Hash_table *hash)
{
  size_t symbols = 0;
  size_t len;

  for (char *line = NULL; getline(&line, &len, stdin) != -1; line = NULL) {
    char *symbol = NULL, *p = line;
    for (char *end; (symbol = get_symbol(p, &end)); p = end) {
      struct freq_symbol fw2 = {symbol, 0};
      // Temporarily insert a NUL to make the symbol a string
      char c = *end;
      *end = '\0';
      freq_symbol_t fw = hash_lookup(hash, &fw2);
      if (fw) {
        fw->count++;
      } else {
        symbols++;
        fw = XMALLOC(struct freq_symbol);
        size_t symlen = end - symbol;
        *fw = (struct freq_symbol) {.symbol = xmalloc(symlen + 1), .count = 1};
        strncpy(fw->symbol, symbol, symlen);
        fw->symbol[symlen] = '\0';
        assert(hash_insert(hash, fw));
      }
      *end = c; // Restore the overwritten character
    }
    free(line);
  }

  return symbols;
}

// Process a file
static void
process(const char *name)
{
  // Read file into symbol table
  Hash_table *hash = hash_initialize(256, NULL, symbolhash, symboleq, NULL);
  size_t symbols = read_symbols(hash);
  if (!args_info.nocount_given)
    fprintf(stderr, "%s: %zd symbols\n", name, symbols);

  // Print out symbol data
  for (freq_symbol_t fw = hash_get_first(hash); fw != NULL; fw = hash_get_next(hash, fw)) {
    printf("%s", fw->symbol);
    if (!args_info.nocount_given)
      printf(" %zd", fw->count);
    putchar('\n');
  }
}

static _Noreturn void
die(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  fprintf(stderr, PACKAGE ": ");
  vfprintf(stderr, fmt, ap);
  putc('\n', stderr);
  va_end(ap);
  exit(1);
}

int
main(int argc, char *argv[])
{
  setlocale(LC_ALL, "");

  // Process command-line options
  if (cmdline_parser(argc, argv, &args_info) != 0)
    exit(1);
  if (args_info.help_given)
    cmdline_parser_print_help();
  if (args_info.version_given)
    cmdline_parser_print_version();

  // Compile regex
  int err = regcomp(&symbol_re, args_info.symbol_arg, REG_EXTENDED);
  if (err != 0) {
    size_t errlen = regerror(err, &symbol_re, NULL, 0);
    char *errbuf = xmalloc(errlen);
    regerror(err, &symbol_re, errbuf, errlen);
    die("%s", errbuf);
  }

  // Process input
  if (args_info.inputs_num == 0)
    process("-");
  else
    for (unsigned i = 0; i < args_info.inputs_num; i++) {
      if (strcmp(args_info.inputs[i], "-") != 0) {
        if (!freopen(args_info.inputs[i], "r", stdin))
          die("cannot open `%s'", args_info.inputs[i]);
      }
      process(args_info.inputs[i]);
      fclose(stdin);
      if (i < (unsigned)args_info.inputs_num - 1)
        putchar('\n');
    }

  return EXIT_SUCCESS;
}
