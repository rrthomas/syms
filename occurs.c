/* occurs
 * Counts the number of occurrences of each symbol in a text file
 * Reuben Thomas (rrt@sc3d.org)
 * vv0.1-0.33 (BASIC) 25/1/92-23/11/97
 * v0.4 25/11/97; v0.41 28/11/97; v0.42 18/2/98; v0.43 28/7/99;
 * v0.44 29/7/99; v0.45 27/11/99; v0.50 17/1/00; v0.51 20/1/00;
 * v0.52 23/10/00; v0.53 28/10/00; v0.54 29/10/00; v0.55 7/12/00;
 * v0.56 10/2/01; v0.57 12/3/01; v0.58 30/6/01; v0.59 15/1/05

 * Eric Hutton's (bookman@rmplc.co.uk) comments on the BASIC version
 * caused me to translate it to C and improve it.
*/


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <locale.h>
#include <getopt.h>

/* RRTLib headers */
#include <rrt/string.h>
#include <rrt/hash.h>
#include <rrt/except.h>


char *progName= "occurs";

#define DEBUG(x)  if (debug) warn(x);

typedef unsigned char uchar;

uchar letter[UCHAR_MAX];  /* table of flags denoting if the corresponding
			   character is a letter */

int counts= FALSE;  /* If true, output sorted by frequency; otherwise in
                       alphabetic order */
int freqs= TRUE;    /* If true, show the frequencies */
int symbol= 2;  /* 0 = a word is any space-delimited sequence
                   1 = a word is an alphanumeric and underline sequence
                   2 = a word is an alphabetic sequence; words are lowercased
                   3 = a word is an SGML tag; words are lowercased */
int debug= FALSE;   /* If true, print debugging information */

/* Type to hold word-frequency pairs */
typedef struct { uchar *word; size_t count; } FreqWord;


/* Print help and exit */
void
help(void)
{
    printf(
    "occurs v0.59 (15 Jan 2005) by Reuben Thomas (rrt@sc3d.org)\n"
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
    "  -d --debug         increase debugging level\n"
    "  -h -? --help       display this help\n"
    );

    exit(EXIT_SUCCESS);
}

/* Options table */
struct option longopts[]= {
    { "counts",      0, NULL, 'c' },
    { "lexical",     0, NULL, 'l' },
    { "freqs",       0, NULL, 'f' },
    { "nofreqs",     0, NULL, 'n' },
    { "any",         0, NULL, 'a' },
    { "identifiers", 0, NULL, 'i' },
    { "words",       0, NULL, 'w' },
    { "tags",        0, NULL, 't' },
    { "debug",       0, NULL, 'd' },
    { "help",        0, NULL, 'h' },
    { 0, 0, 0, 0 }
};


/* Parse the command-line options; return the number of the first non-option
   argument */
int
getopts(int argc, char *argv[])
{
    int opt;

    while ((opt= getopt_long(argc, argv, "clfnaiwtdh", longopts, NULL))
            != EOF) {
        switch (opt) {
            case 'c': counts= TRUE;   break;
            case 'l': counts= FALSE;  break;
            case 'f': freqs= TRUE;    break;
            case 'n': freqs= FALSE;   break;
            case 'a': symbol= 0;      break;
            case 'i': symbol= 1;      break;
            case 'w': symbol= 2;      break;
            case 't': symbol= 3;      break;
            case 'd': ++debug;        break;
            case 'h':
	    case '?': help();         break;
        }
    }

    return optind;
}


/* Set up the letter[] flags array according to the value of symbol */
void
initLetters(void)
{
    int i;

    for (i= 0; i < UCHAR_MAX; i++)
        switch (symbol) {
            case 0:  letter[i]= TRUE; break;
            case 1:  letter[i]= isalnum(i) != 0; break;
            case 2:
            case 3:  letter[i]= isalpha(i) != 0; break;
        }
    if (symbol == 1) letter['_']= TRUE;

    if (debug > 1) {
	warn("Character table:");
	for (i= 0; i < UCHAR_MAX; i++)
	    if (isalpha(i)) warn("%c, %d : %d", i, i, letter[i]);
    }
}


/* Read a single word */
uchar
*getWord(void)
{
    uchar c, word[BUFSIZ], *ret;
    int i;

    do { c= getchar(); } while (!letter[c] && !feof(stdin));
    if (feof(stdin)) return NULL;

    i= 0;
    do {
        word[i++]= c;
        c= getchar();
    } while (letter[c] && !feof(stdin) && i < BUFSIZ);
    if (i == BUFSIZ) throw("word too long");
    word[i]= '\0';

    if (symbol >= 2)
        for (i = 0; word[i]; i++) word[i]= tolower(word[i]);

    ret= (uchar *)excCalloc(i + 1, sizeof(uchar));
    strcpy(ret, word);
    if (debug > 1) fprintf(stderr, "%s ", ret);
    return ret;
}


/* Read the file into a hash table */
size_t
readWords(HashTable *table)
{
    size_t words= 0, *freq;
    uchar *word;

    while (!feof(stdin)) {
        if (symbol == 3) while (getchar() != '<' && !feof(stdin));
        if (!feof(stdin)) {
            word= getWord();
            if (word) {
                if (freq= (size_t *)hashFind(table, word))
                    (*freq)++;
                else {
                    words++;
                    freq= excCalloc(1, sizeof(size_t));
                    *freq= 1;
                    hashInsert(table, word, freq);
                }
            }
        }
    }

    return words;
}


/* Turn a hash table into a linear array */
FreqWord *
flatten(HashTable *table, size_t words)
{
    size_t i, j;
    HashNode *p, *q;
    FreqWord (*array)[]= excCalloc(words, sizeof(FreqWord));

    for (i= 0, j= 0; i < table->size; i++)
        for (p= table->thread[i]; p != NULL; p= q, j++) {
            (*array)[j].word= p->key;
            (*array)[j].count= *(size_t *)p->body;
            q= p->next;
            free(p->body);
            free(p);
        }
    free(table->thread);
    free(table);

    return *array;
}


/* Compare a FreqWord on the count field */
int
countCmp(const void *keyval, const void *datum)
{
    size_t k= ((FreqWord *)keyval)->count, d= ((FreqWord *)datum)->count;

    return (k < d ? 1 : (k == d ? 0 : -1));
}

/* Compare a FreqWord on the word field */
int
wordCmp(const void *keyval, const void *datum)
{
    uchar *k= ((FreqWord *)keyval)->word, *d= ((FreqWord *)datum)->word;

    return strcoll((char *)k, (char *)d);
}


/* Process a file */
void
process(char *name)
{
    size_t words, i;
    HashTable *table= hashNew(4096, hashStrHash, hashStrcmp);
    FreqWord *array;

    DEBUG("Reading words");
    words= readWords(table);
    DEBUG("Flattening table");
    array= flatten(table, words);
    DEBUG("Sorting");
    qsort((void *)array, words, sizeof(FreqWord),
        counts ? countCmp : wordCmp);

    warn("%s: %lu words", name, words);
    for (i= 0; i < words; i++) {
        printf("%s", array[i].word);
        if (freqs) printf(" %d", array[i].count);
        putchar('\n');
    }
}


int
main(int argc, char *argv[])
{
    char *loc= setlocale(LC_ALL, "");
    int i= getopts(argc, argv);

    excInit();
    if (debug) warn("Locale: %s\n", loc);
    initLetters();

    if (i < argc)
        for (; i < argc; i++) {
            if (strEq(argv[i], "-")) process("standard input");
            else {
                if (!freopen(argv[i], "r", stdin))
                    throw("can't open %s", argv[i]);
                process(argv[i]);
            }
            fclose(stdin);
            if (i < argc - 1) putchar('\n');
        }
    else
        help();

    return EXIT_SUCCESS;
}
