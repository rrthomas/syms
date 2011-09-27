/* occurs
 * Counts the number of occurrences of each symbol in a text file
 * Reuben Thomas (rrt@sc3d.org)
 */

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
#include "gl_xlist.h"
#include "gl_linked_list.h"

#include "cmdline.h"


static regex_t symbol_re; // regex object corresponding to the regexs above

// Type to hold symbol-frequency pairs
typedef struct freq_symbol {
  const char *symbol;
  size_t count;
} *freq_symbol_t;

struct gengetopt_args_info args_info;

// Compare a freq_symbol on the symbol field
static int
symbolcmp(const void *s1, const void *s2)
{
  const char *s1s = ((freq_symbol_t)s1)->symbol, *s2s = ((freq_symbol_t)s2)->symbol;
  return strcoll((const char *)s1s, (const char *)s2s);
}

// Compare a freq_symbol on the frequency field
static int
freqcmp(const void *s1, const void *s2)
{
  size_t s1c = ((freq_symbol_t)s1)->count, s2c = ((freq_symbol_t)s2)->count;
  return s1c < s2c ? -1 : (s1c == s2c ? 0 : 1);
}

static size_t
symbolhash(const void *v, size_t n)
{
  return hash_string(((freq_symbol_t) v)->symbol, n);
}

static bool
symboleq (const void *v, const void *w)
{
  return strcmp(((freq_symbol_t) v)->symbol, ((freq_symbol_t) w)->symbol) == 0;
}

static int (*comparer)(const void *s1, const void *s2);

static const char *
get_symbol(const char *s, size_t *n)
{
  regmatch_t match[1];
  if (regexec(&symbol_re, s, 1, match, 0) != 0)
    return NULL;
  *n = match[0].rm_eo;
  char *w = xmalloc(*n + 1);
  size_t len = *n - match[0].rm_so;
  strncpy(w, s + match[0].rm_so, len);
  w[len] = '\0';
  if (args_info.lower_given)
    for (size_t i = 0; i < len; i++)
      w[i] = tolower(w[i]);
  return w;
}

// Read the file into a list
static size_t
read_symbols(Hash_table *hash)
{
  size_t symbols = 0;
  ssize_t len;
  char *line = NULL;

  while (getline(&line, &len, stdin) != -1) {
    const char *symbol = NULL;
    for (size_t n = 0; symbol = get_symbol(line, &n); line += n) {
      struct freq_symbol fw2 = {symbol, 0};
      freq_symbol_t fw = hash_lookup(hash, &fw2);
      if (fw)
        fw->count++;
      else {
        symbols++;
        fw = XZALLOC(struct freq_symbol);
        *fw = (struct freq_symbol) {.symbol = symbol, .count = 1};
        assert(hash_insert(hash, fw));
      }
    }
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

  // Flatten and sort symbol table
  gl_list_t list = gl_list_create_empty(GL_LINKED_LIST,
                                        NULL, NULL, NULL, false);
  for (freq_symbol_t fw = hash_get_first(hash); fw != NULL; fw = hash_get_next(hash, fw))
    gl_sortedlist_add(list, symbolcmp, fw);

  // Print out symbol data
  if (!args_info.nocount_given)
    fprintf(stderr, "%s: %lu symbols\n", name, (long unsigned)symbols);
  for (size_t i = 0; i < symbols; i++) {
    freq_symbol_t fw = (freq_symbol_t)gl_list_get_at(list, i);
    printf("%s", fw->symbol);
    if (!args_info.nocount_given)
      printf(" %zd", fw->count);
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
