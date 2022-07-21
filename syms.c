// syms
// (c) Reuben Thomas <rrt@sc3d.org>


#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <locale.h>
#include <regex.h>
#include "xalloc.h"
#include "error.h"
#include "quote.h"

#include "cmdline.h"


struct gengetopt_args_info args_info;

static const char *
get_symbol(regex_t *re, const char *s, const char **end)
{
  regmatch_t match[1];
  if (regexec(re, s, 1, match, 0) != 0)
    return NULL;
  *end = s + match[0].rm_eo;
  return s + match[0].rm_so;
}

int
main(int argc, char *argv[])
{
  setlocale(LC_ALL, "");

  // Process command-line options
  if (cmdline_parser(argc, argv, &args_info) != 0)
    exit(EXIT_FAILURE);
  if (args_info.help_given)
    cmdline_parser_print_help();
  if (args_info.version_given)
    cmdline_parser_print_version();

  // Compile regex
  regex_t re;
  int err = regcomp(&re, args_info.symbol_arg, REG_EXTENDED);
  if (err != 0) {
    size_t errlen = regerror(err, &re, NULL, 0);
    char *errbuf = xmalloc(errlen);
    regerror(err, &re, errbuf, errlen);
    error(EXIT_FAILURE, errno, "%s", errbuf);
  }

  // Process input
  for (unsigned i = 0; i <= args_info.inputs_num; i++) {
    if (i < args_info.inputs_num && strcmp(args_info.inputs[i], "-") != 0) {
      if (!freopen(args_info.inputs[i], "r", stdin))
        error(EXIT_FAILURE, errno, "cannot open %s", quote(args_info.inputs[i]));
    }
    size_t len;
    for (char *line = NULL; getline(&line, &len, stdin) != -1; line = NULL) {
      const char *symbol = NULL, *p = line;
      for (const char *end; (symbol = get_symbol(&re, p, &end)); p = end)
        printf("%.*s\n", (int)(end - symbol), symbol);
      free(line);
    }
    fclose(stdin);
    if (i < (unsigned)args_info.inputs_num - 1)
      putchar('\n');
  }

  return EXIT_SUCCESS;
}
