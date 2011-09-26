/* occurs
 * Counts the number of occurrences of each symbol in a text file
 * Reuben Thomas (rrt@sc3d.org)
 */

/* FIXME: Use POSIX regex for symbol matching. */
/* FIXME: Cope with wide character encodings. */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <locale.h>
#include <getopt.h>
#include "xalloc.h"
#include "gl_xlist.h"
#include "gl_linked_list.h"

#include "cmdline.h"


typedef unsigned char uchar;

uchar letter[UCHAR_MAX];  /* table of flags denoting if the corresponding
                           character is a letter */

int counts = false;  /* If true, output sorted by frequency; otherwise in
                        alphabetic order */
int freqs = true;    /* If true, show the frequencies */
int symbol = 2;  /* 0 = any space-delimited sequence
                    1 = an alphanumeric and underline sequence
                    2 = an alphabetic sequence; words are lowercased
                    3 = an SGML tag; words are lowercased */

/* Type to hold word-frequency pairs */
struct freq_word {
  uchar *word;
  size_t count;
};
typedef struct freq_word *freq_word_t;


/* Set up the letter[] flags array according to the value of symbol */
static void
init_letters(void)
{
  unsigned i;

  for (i = 0; i < UCHAR_MAX; i++)
    switch (symbol) {
    case 0:
      letter[i] = true;
      break;
    case 1:
      letter[i] = isalnum(i) != 0;
      break;
    case 2:
    case 3:
      letter[i] = isalpha(i) != 0;
      break;
    }
  if (symbol == 1)
    letter['_']= true;
}

/* Read a single word */
static uchar *
get_word(void)
{
  /* FIXME: Remove limit on length of word */
  uchar c, word[BUFSIZ], *ret;
  int i;

  do {
    c = getchar();
  } while (!letter[c] && !feof(stdin));
  if (feof(stdin))
    return NULL;

  i = 0;
  do {
    word[i++]= c;
    c = getchar();
  } while (letter[c] && !feof(stdin) && i < BUFSIZ);
  if (i == BUFSIZ) {
    fprintf(stderr, "word too long");
    exit(1);
  }
  word[i]= '\0';

  if (symbol >= 2)
    for (i = 0; word[i]; i++) word[i]= tolower(word[i]);

  ret = (uchar *)xcalloc(i + 1, sizeof(uchar));
  strcpy(ret, word);
  return ret;
}

/* Compare a freq_word on the word field */
static int
wordcmp(const void *keyval, const void *datum)
{
  uchar *k = ((freq_word_t)keyval)->word, *d = ((freq_word_t)datum)->word;

  return strcoll((char *)k, (char *)d);
}

/* Read the file into a list */
static size_t
read_words(gl_list_t list)
{
  size_t words = 0;
  uchar *word;
  freq_word_t fw;

  while (!feof(stdin)) {
    if (symbol == 3)
      while (getchar() != '<' && !feof(stdin))
        ;
    if (!feof(stdin)) {
      if ((word = get_word())) {
        struct freq_word fw2 = {word, 0};
        size_t i = gl_sortedlist_indexof(list, wordcmp, &fw2);
        if (i != (size_t)-1) {
          fw = (freq_word_t)gl_list_get_at(list, i);
          fw->count++;
        } else {
          words++;
          fw = XZALLOC(struct freq_word);
          fw->word = word;
          fw->count = 1;
          gl_sortedlist_add(list, wordcmp, fw);
        }
      }
    }
  }

  return words;
}

/* Process a file */
static void
process(char *name)
{
  size_t words, i;
  gl_list_t list = gl_list_create_empty(GL_LINKED_LIST,
                                        NULL, NULL, NULL, false);

  words = read_words(list);

  fprintf(stderr, "%s: %lu words\n", name, (long unsigned)words);
  for (i = 0; i < words; i++) {
    freq_word_t fw = (freq_word_t)gl_list_get_at(list, i);
    printf("%s", fw->word);
    if (freqs)
      printf(" %zd", fw->count);
    putchar('\n');
  }
}

int
main(int argc, char *argv[])
{
  struct gengetopt_args_info args_info;
  char *loc = setlocale(LC_ALL, "");
  unsigned i;

  if (cmdline_parser(argc, argv, &args_info) != 0)
    exit(1);
  if (args_info.help_given || argc == 1)
    cmdline_parser_print_help();
  if (args_info.version_given)
    cmdline_parser_print_version();

  init_letters();

  for (i = 0; i < args_info.inputs_num; i++) {
    if (strcmp(argv[i], "-") != 0) {
      if (!freopen(argv[i], "r", stdin)) {
        fprintf(stderr, "cannot open `%s'", argv[i]);
        exit(1);
      }
    }
    process(argv[i]);
    fclose(stdin);
    if (i < (unsigned)argc - 1)
      putchar('\n');
  }

  return EXIT_SUCCESS;
}
