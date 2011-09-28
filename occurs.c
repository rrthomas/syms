// occurs
// Counts the number of occurrences of each symbol in a text file
// Reuben Thomas (rrt@sc3d.org)


// FIXME: Cope with wide character encodings.

#include <config.h>

#include <assert.h>
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

// Compare a freq_symbol on the symbol field
static int
symbolcmp(const void *s1, const void *s2)
{
  char *s1s = ((const struct freq_symbol *)s1)->symbol;
  char *s2s = ((const struct freq_symbol *)s2)->symbol;
  return strcoll((const char *)s1s, (const char *)s2s);
}

// Compare a freq_symbol on the frequency field
static int
freqcmp(const void *s1, const void *s2)
{
  size_t s1c = ((const struct freq_symbol *)s1)->count, s2c = ((const struct freq_symbol *)s2)->count;
  return s1c < s2c ? -1 : (s1c == s2c ? 0 : 1);
}

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
  char *w = s + match[0].rm_so;
  if (args_info.lower_given)
    for (char *p = w; p < *end; p++)
      *p = tolower(*p);
  return w;
}

// Read the file into a list
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

static int (*comparer)(const void *s1, const void *s2);

// Process a file
static void
process(const char *name)
{
  // Read file into symbol table
  Hash_table *hash = hash_initialize(256, NULL, symbolhash, symboleq, NULL);
  size_t symbols = read_symbols(hash);
  if (!args_info.nocount_given)
    fprintf(stderr, "%s: %lu symbols\n", name, (long unsigned)symbols);

  // Flatten and sort symbol table
  freq_symbol_t *list = xmalloc(sizeof(freq_symbol_t) * symbols);
  size_t i = 0;
  for (freq_symbol_t fw = hash_get_first(hash); fw != NULL; fw = hash_get_next(hash, fw), i++)
    list[i] = fw;
  if (args_info.sort_given)
    qsort(list, symbols, sizeof(freq_symbol_t), comparer);

  // Print out symbol data
  for (i = 0; i < symbols; i++) {
    printf("%s", list[i]->symbol);
    if (!args_info.nocount_given)
      printf(" %zd", list[i]->count);
    putchar('\n');
  }
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

  // Set sort order
  if (args_info.sort_given) {
    if (strcmp(args_info.sort_arg, "lexical") == 0)
      comparer = symbolcmp;
    else if (strcmp(args_info.sort_arg, "frequency") == 0)
      comparer = freqcmp;
  }

  // Compile regex
  char *s;
  assert(asprintf(&s, "%s(%s)%s", args_info.left_arg, args_info.symbol_arg, args_info.right_arg) > 0);
  int err = regcomp(&symbol_re, s, REG_EXTENDED);
  if (err != 0) {
    size_t errlen = regerror(err, &symbol_re, NULL, 0);
    char *errbuf = xmalloc(errlen);
    regerror(err, &symbol_re, errbuf, errlen);
    fprintf(stderr, PACKAGE ": %s\n", errbuf);
  }

  // Process input
  if (args_info.inputs_num == 0)
    process("-");
  else
    for (unsigned i = 0; i < args_info.inputs_num; i++) {
      if (strcmp(args_info.inputs[i], "-") != 0) {
        if (!freopen(args_info.inputs[i], "r", stdin)) {
          fprintf(stderr, PACKAGE ": cannot open `%s'\n", args_info.inputs[i]);
          exit(1);
        }
      }
      process(args_info.inputs[i]);
      fclose(stdin);
      if (i < (unsigned)args_info.inputs_num - 1)
        putchar('\n');
    }

  return EXIT_SUCCESS;
}
