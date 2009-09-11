/* occurs
 * Counts the number of occurrences of each symbol in a text file
 * Reuben Thomas (rrt@sc3d.org)
 */

/* FIXME: Use POSIX regex for symbol matching. */
/* FIXME: Use gengetopt options-parsing code to eliminate redundancy. */
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
#include "gl_linked_list.h"


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


/* Print help and exit */
static void
help(void)
{
  fprintf(stderr,
          /* FIXME: Generate name below */
          PACKAGE_NAME " " VERSION " by Reuben Thomas (" PACKAGE_BUGREPORT ")\n"
          "Counts the number of occurrences of each symbol in a file\n"
          "\n"
          "Usage: occurs [options] file...\n"
          "  In the file list, \'-\' means read from standard input\n"
          "\n"
          "Options:\n"
          "  -c --counts        give output sorted by frequency\n"
          "  -l --lexical       give output in lexical order [default]\n"
          "  -f --freqs         show the frequencies [default]\n"
          "  -n --nofreqs       don't show the frequencies\n"
          "  -a --any           symbols consist of non-white-space characters\n"
          "  -i --identifiers   symbols consist of alphanumerics and underscores\n"
          "  -w --words         symbols consist of letters, and are lowercased "
          "[default]\n"
          "  -t --tags          symbols are SGML tags\n"
          "  -h -? --help       display this help\n"
          );

  exit(EXIT_SUCCESS);
}

/* Options table */
struct option longopts[] = {
  { "counts",      0, NULL, 'c' },
  { "lexical",     0, NULL, 'l' },
  { "freqs",       0, NULL, 'f' },
  { "nofreqs",     0, NULL, 'n' },
  { "any",         0, NULL, 'a' },
  { "identifiers", 0, NULL, 'i' },
  { "words",       0, NULL, 'w' },
  { "tags",        0, NULL, 't' },
  { "help",        0, NULL, 'h' },
  { 0, 0, 0, 0 }
};


/* Parse the command-line options; return the number of the first non-option
   argument */
static int
getopts(int argc, char *argv[])
{
  int opt;

  while ((opt= getopt_long(argc, argv, "clfnaiwtdh", longopts, NULL))
         != EOF) {
    switch (opt) {
    case 'c': counts = true;   break;
    case 'l': counts = false;  break;
    case 'f': freqs = true;    break;
    case 'n': freqs = false;   break;
    case 'a': symbol = 0;      break;
    case 'i': symbol = 1;      break;
    case 'w': symbol = 2;      break;
    case 't': symbol = 3;      break;
    case 'h':
    case '?': help();          break;
    }
  }

  return optind;
}

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
      printf(" %d", fw->count);
    putchar('\n');
  }
}

int
main(int argc, char *argv[])
{
  char *loc = setlocale(LC_ALL, "");
  int i = getopts(argc, argv);

  init_letters();

  if (i < argc)
    for (; i < argc; i++) {
      if (strcmp(argv[i], "-") != 0) {
        if (!freopen(argv[i], "r", stdin)) {
          fprintf(stderr, "cannot open `%s'", argv[i]);
          exit(1);
        }
      }
      process(argv[i]);
      fclose(stdin);
      if (i < argc - 1)
        putchar('\n');
    }
  else
    help();

  return EXIT_SUCCESS;
}
