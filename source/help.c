// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <stdio.h>
#include <string.h>
#include "shared.h"
#include "help.h"
#include "set.h"       // For linesize and prcomment
#include "shell.h"     // For outfile
#include "mac.h"       // For helpFile
#include "cmdnames.h"  // For cmd_list array
#include "monitor.h"   // For print functions
#include "human_io.h"  // For get_defint
#include "input.h"     // For topen

// Help file format markers
constexpr char FIRST[] = "@";
constexpr char FIRSTC = '@';
constexpr char SECOND[] = "~";
constexpr char SECONDC = '~';

static boolean displayItem(FILE* helpfil)
{
    char str[100];

    while (!feof(helpfil))
    {
        if (fgets(str, 100, helpfil) == NULL)
            break;
        if (strncmp(str, FIRST, 1) == 0) // bump out if next topic is found
            return TRUE;
        if (strncmp(str, SECOND, 1) == 0) // bump out if next topic is found
            return TRUE;
        fprintnew(outfile, "%s", str);
    }
    return (FALSE);
}

static void displayTopic(FILE* helpfil, int secn, int topn)
{
    char str[100];
    boolean found;

    if ((secn == 0) || (topn == 0))
        return;
    rewind(helpfil);
    found = FALSE;
    while (!feof(helpfil))
    {
        if (fgets(str, 100, helpfil) == NULL)
            break;
        if (strncmp(str, FIRST, 1) == 0)
            secn--;
        if ((strncmp(str, SECOND, 1) == 0) && (secn == 0))
        {
            topn--;
            if (topn == 0)
            {
                fprint(outfile, "\n");
                fprintnew(outfile, "%s", str + 1);
                displayItem(helpfil);
                found = TRUE;
            }
        }
    }
    if (!found)
        prerror("; no help on this topic\n");
}

// the following routine looks up "topic" in the help file (it does NOT reset
// the file.  The section and topic numbers are set: if no section is found,
// secn is set to 0.  If "topic" is a main topic, then "topn" is set to 0.

static void findTopic(FILE* helpfil, char* topic, int* secn, int* topn)
{
    char str[100];

    *secn = 0;
    *topn = 0;
    while (!feof(helpfil))
    {
        if (fgets(str, 100, helpfil) == NULL)
            break;
        if (*str == FIRSTC)
        {
            (*secn)++;
            *topn = 0;
        }
        else if (*str == SECONDC)
            (*topn)++;
        else
            continue;
        if (strncmp(str + 1, topic, strlen(topic)) == 0)
            return;
    }
}

static boolean nextItem(FILE* helpfil, char* str, const char* headerstr)
{
    while (!feof(helpfil))
    {
        if (fgets(str, 100, helpfil) == NULL)
            break;
        if (strncmp(str, headerstr, strlen(headerstr)) == 0)
            return (TRUE);
        if (strncmp(str, FIRST, 1) == 0) // bump out if next topic is found
            return (FALSE);
    }
    return (FALSE);
}

static void toSection(FILE* helpfil, char* str, int secn)
{
    rewind(helpfil);
    while (secn > 0)
    {
        nextItem(helpfil, str, FIRST);
        secn--;
    }
}

static int menuMain(FILE* helpfil, const char* level)
{
    int nitems, n;
    char s[100];

    nitems = 0;
    while (nextItem(helpfil, s, level))
    {
        nitems++;
        fprintnew(outfile, "(%d) %s", nitems, s + 1);
    }
    fprintnew(outfile, "\n");

    n = get_defint("which item (give its number, <return> to leave help)", 0);
    if ((n < 0) || (n > nitems))
    {
        prerror("; topic number out of range\n");
        return (0);
    }
    return (n);
}

static int menuTopics(FILE* helpfil, int secn)
{
    int topn;
    char str[100];

    if (secn == 0)
        return (0);
    toSection(helpfil, str, secn);
    printnew("topic: %s\n", str + 1);
    topn = menuMain(helpfil, SECOND);
    return (topn);
}

void help_cmd(int argc, char* argv[])
{
    FILE* helpfil;
    int secn, topn;

    helpfil = topen(helpFile, "r");
    if (helpfil == NULL)
    {
        prerror("; Sorry, but I can't find the help file...\n");
        return;
    }

    if (argc == 1)
    { // do main menu
        secn = menuMain(helpfil, FIRST);
        topn = menuTopics(helpfil, secn);
        displayTopic(helpfil, secn, topn);
        fclose(helpfil);
        return;
    }
    findTopic(helpfil, argv[1], &secn, &topn);
    if (secn == 0)
    {
        prerror("; no help on that topic\n");
        fclose(helpfil);
        return;
    }
    if (topn == 0)
        topn = menuTopics(helpfil, secn);
    displayTopic(helpfil, secn, topn);
    fclose(helpfil);
}

// the following command lists all the commands in alpha order

void commands_cmd(void)
{
    int nelems, maxlength, n, m, i, j;
    int nlines, ncolumns;
    const char* s;
    char str[30]; // large enough to hold each command and a blank

    if (prcomment > 0)
        linesize -= 2;
    nelems = 0;
    maxlength = 0;
    for (i = 0; *(s = cmd_list[i].name) != '\0'; i++)
    {
        n = (int)strlen(s);
        if (n > maxlength)
            maxlength = n;
        nelems++;
    }
    ncolumns = linesize / (maxlength + 1);
    if (ncolumns == 0)
        ncolumns = 1; // if linesize was set too small...
    nlines = (nelems - 1) / ncolumns + 1;

    for (i = 0; i < nlines; i++)
    {
        print("\n");
        newline();
        for (j = 0; j < ncolumns; j++)
        {
            m = i + nlines * j;
            if (m >= nelems)
                break;
            sprintf(str, "%-*s ", maxlength, cmd_list[m].name);
            print("%s", str);
        }
    }
    print("\n");
    if (prcomment > 0)
        linesize += 2;
}

void helpfile_cmd(int argc, char* argv[])
{
    char s[120];
    FILE* fil;

    if (argc != 2)
    {
        printnew("help_file <file name>\n");
        return;
    }

    fil = topen(argv[1], "r");
    if (fil == NULL)
    {
        prerror("; file %s not found\n", argv[1]);
        return;
    }

    while (nextItem(fil, s, ";;;"))
        fprintnew(outfile, "%s", s + 3);

    fclose(fil);
}
