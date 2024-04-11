/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include "mtypes.h"

// void displayItem (FILE *helpfil);
// void displayTopic (FILE *helpfil, int secn, int topn);
// void findTopic (FILE *helpfil, char *topic, int *secn, int *topn);
// int menuTopics (FILE *helpfil, int secn);
// boolean nextItem (FILE *helpfil, char *str, char *headerstr);
void toSection (FILE *helpfil, char *str, int secn);
// int menuMain (FILE *helpfil, char *level);
// void help_cmd (int argc, char *argv[]);
// void commands_cmd ();
// void helpfile_cmd (int argc, char *argv[]);

extern int linesize;
extern int prcomment;
extern FILE *outfile;

extern char *helpFile;

#define FIRST "@"
#define FIRSTC '@'
#define SECOND "~"
#define SECONDC '~'

void displayItem (FILE *helpfil)
{
    char str[100];

    while (NOT feof(helpfil)) {
        if (fgets(str, 100, helpfil) IS NULL) break;
        if (strncmp(str, FIRST, 1) IS 0)  /* bump out if next topic is found */
            return;
        if (strncmp(str, SECOND, 1) IS 0) /* bump out if next topic is found */
            return;
        fprintnew(outfile, "%s", str);
    }
    return;
}

void displayTopic (FILE *helpfil, int secn, int topn)
{
    char str[100];
    boolean found;

    if ((secn IS 0) OR (topn IS 0)) return;
    rewind(helpfil);
    found = FALSE;
    while (NOT feof(helpfil)) {
        if (fgets(str, 100, helpfil) IS NULL) break;
        if (strncmp(str, FIRST, 1) IS 0)
            secn--;
        if ((strncmp(str, SECOND, 1) IS 0) AND (secn IS 0)) {
            topn--;
            if (topn IS 0) {
                fprint(outfile, "\n");
                fprintnew(outfile, "%s", str+1);
                displayItem(helpfil);
                found = TRUE;
            }
        }
    }
    if (NOT found)
        prerror("; no help on this topic\n");
}

/* the following routine looks up "topic" in the help file (it does NOT reset
   the file.  The section and topic numbers are set: if no section is found,
   secn is set to 0.  If "topic" is a main topic, then "topn" is set to 0.
*/

void findTopic (FILE *helpfil, char *topic, int *secn, int *topn)
{
    char str[100];

    *secn = 0;
    *topn = 0;
    while (NOT feof(helpfil)) {
        if (fgets(str, 100, helpfil) IS NULL) break;
        if (*str IS FIRSTC) {
            (*secn)++;
            *topn = 0;
        } else if (*str IS SECONDC)
            (*topn)++;
        else
            continue;
        if (strncmp(str+1, topic, strlen(topic)) IS 0)
            return;
    }
}

int menuTopics (FILE *helpfil, int secn)
{
    int topn;
    char str[100];

    if (secn IS 0) return(0);
    toSection(helpfil, str, secn);
    printnew("topic: %s\n", str+1);
    topn = menuMain(helpfil, SECOND);
    return(topn);
}

boolean nextItem (FILE *helpfil, char *str, char *headerstr)
{
    while (NOT feof(helpfil)) {
        if (fgets(str, 100, helpfil) IS NULL) break;
        if (strncmp(str, headerstr, strlen(headerstr)) IS 0)
            return(TRUE);
        if (strncmp(str, FIRST, 1) IS 0)  /* bump out if next topic is found */
            return(FALSE);
    }
    return(FALSE);
}

void toSection (FILE *helpfil, char *str, int secn)
{
    rewind(helpfil);
    while (secn > 0) {
        nextItem(helpfil, str, FIRST);
        secn--;
    }
}

int menuMain (FILE *helpfil, char *level)
{
    int nitems, n;
    char s[100];

    nitems = 0;
    while (nextItem(helpfil, s, level)) {
        nitems++;
        fprintnew(outfile, "(%d) %s", nitems, s+1);
    }
    fprintnew(outfile, "\n");

    n = get_defint("which item (give its number, <return> to leave help)",
                   0);
    if ((n < 0) OR (n > nitems)) {
        prerror("; topic number out of range\n");
        return(0);
    }
    return(n);
}

void help_cmd (int argc, char *argv[])
{
    FILE *helpfil, *topen();
    int secn, topn;

    helpfil = topen(helpFile, "r");
    if (helpfil IS NULL) {
        prerror("; Sorry, but I can't find the help file...\n");
        return;
    }

    if (argc IS 1) {    /* do main menu */
        secn = menuMain(helpfil, FIRST);
        topn = menuTopics(helpfil, secn);
        displayTopic(helpfil, secn, topn);
        return;
    }
    findTopic(helpfil, argv[1], &secn, &topn);
    if (secn IS 0) {
        prerror("; no help on that topic\n");
        return;
    }
    if (topn IS 0)
        topn = menuTopics(helpfil, secn);
    displayTopic(helpfil, secn, topn);
}

/* the following command lists all the commands in alpha order */

void commands_cmd ()
{
    extern cmd_rec cmd_list[];

    int nelems, maxlength, n, m, i, j;
    int nlines, ncolumns;
    char *s, str[30];  /* large enough to hold each command and a blank */

    if (prcomment > 0) linesize -= 2;
    nelems = 0;
    maxlength = 0;
    for (i=0; *(s=cmd_list[i].name) ISNT '\0'; i++) {
        n = strlen(s);
        if (n > maxlength)
            maxlength = n;
        nelems++;
    }
    ncolumns = linesize/(maxlength+1);
    if (ncolumns IS 0) ncolumns = 1;  /* if linesize was set too small... */
    nlines = (nelems-1)/ncolumns + 1;

    for (i=0; i<nlines; i++) {
        print("\n");
        newline();
        for (j=0; j<ncolumns; j++) {
            m = i + nlines*j;
            if (m >= nelems) break;
            sprintf(str, "%-*s ", maxlength, cmd_list[m].name);
            print("%s", str);
        }
    }
    print("\n");
    if (prcomment > 0) linesize += 2;
}

void helpfile_cmd (int argc, char *argv[])
{
    char s[120];
    FILE *fil, *topen();

    if (argc ISNT 2) {
        printnew("help_file <file name>\n");
        return;
    }

    fil = topen(argv[1], "r");
    if (fil IS NULL) {
        prerror("; file %s not found\n", argv[1]);
        return;
    }

    while (nextItem(fil, s, ";;;"))
        fprintnew(outfile, "%s", s+3);

    fclose(fil);
}
