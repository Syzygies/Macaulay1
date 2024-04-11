/* Copyright 1989 Dave Bayer and Mike Stillman. All rights reserved. */
#include <stdio.h>
#include <ctype.h>
extern int level;
extern int echo;
extern int prlevel;
extern int nlines;

// static void set_path (char *path);
// static void set_cdir (char *cdir);
// void init_path ();
// void path_cmd (int argc, char *argv[]);
// void cdir_cmd (int argc, char *argv[]);
// static char *file_name (char *name, char *prefix, int prelen);
// FILE *topen (char *fileName, char *mode);
// void sniff_prlevel (char *linebuf);
// void unsniff_prlevel ();
// void break_voice ();
void drop_voice ();
// int change_level ();
// void prompt ();
// int get_chars (FILE *fd, char *a);
// void raw_line ();
// void get_line (int *pargc, char ***pargv);
// int open_journal (char *s);
// int debug_voice ();
// int open_tour (char *s);
// void set_names (int ac, char **av);
void subs_names ();
// void get_cmd_line (int *pargc, char ***pargv, char **poutfile);
// void branch (char *label);
// void if_cmd (int argc, char *argv[]);
// void jump_cmd (int argc, char *argv[]);

#define PROMPT1 '%'
#define PROMPT2 '?'

#define EOLN(A) ((A) == '\n' || (A) == '\r' || (A) == EOF)
#ifndef MIN
#define MIN(A, B) ( (A) < (B) ? (A) : (B) )
#endif
#ifndef MAX
#define MAX(A, B) ( (A) > (B) ? (A) : (B) )
#endif
#define VISIBLE(A) ((A) > 32 && (A) < 127)

#define NVOICES 50
#define NLEVELS 12
#define NARGS 40
#define NCHARS 200
#define NNAMES 200
#define VNAMES 600

extern int in_dot_cmd_mode;

char linebuf[NCHARS];
char *argv[NARGS];
int cmd = 0, old_level = -1, cur_voice = -1;
int voice[NLEVELS];
FILE *fd;
FILE *fj;
int jf;
FILE *fstack[NVOICES];
FILE *jstack[NVOICES];
int dotflag[NVOICES];
int jflag[NVOICES];
int inam, nnam, inames[NVOICES], nnames[NVOICES], indnames[NNAMES];
char namespace[VNAMES];
static int top, sniff;

int com = 0; /* was previous cmd line a comment ? */
int remnull = 0; /* was last line a cmd but empty? */

extern char *MclyPath, *MclyCdir;

char colon = '/', comma = ':';

/* MclyPath is predefined in mac.c, and each call to set_path() overrides this
 * def, unless the new def begins with a system dependent comma character
 * (, or :), in which case set_path() appends the new def. init_path() reads
 * the environment variable MacaulayPath, then looks for the file Macaulay.path
 * using whatever path has been built up so far. The path command can then be
 * called either from Macaulay.init, or by the user.
 *
 * MclyCdir is initially NULL unless a local installation defines in mac.c.
 * Each call to set_cdir() overrides previous definition. init_path() also reads
 * the environment variable MacaulayCdir, then looks for the file Macaulay.cdir.
 * The cdir command can then be used to change MclyCdir.
 *
 * If MclyCdir isn't NULL, it is appended to every fopen() in mode "w", and also
 * used as a first try in mode "r". In mode "r", if this first try fails, or if
 * MclyCdir is NULL, then each of the directories specified in MclyPath are tried.
 */

static void set_path (char *path)
{
    int len;
    char *old, *gimmy();

    if (path != NULL) {
        old = MclyPath;
        if (*path == comma) {
            len = strlen(path);
            if (len > 1) {
                len += 1 + strlen(old);
                MclyPath = gimmy(len);
                sprintf (MclyPath, "%s%s", old, path);
            }
        }
        else if (*path != '\0') {
            len = 1 + strlen(path);
            MclyPath = gimmy(len);
            sprintf (MclyPath, "%s", path);
        }
    }
}

static void set_cdir (char *cdir)
{
    int len;
    char *gimmy();

    if (cdir != NULL) {
        if (cdir[0] == '\0'
                || (cdir[0] == '.' && cdir[1] == '\0'))
            MclyCdir = NULL;
        else {
            len = 1 + strlen(cdir);
            MclyCdir = gimmy(len);
            sprintf (MclyCdir, "%s", cdir);
        }
    }
}

void init_path ()
{
#define NN 1024
    char *path, *cdir, *getenv(), paths[NN];
    FILE *fp, *topen();
    int c, i, m;

    path = getenv("MacaulayPath");
    set_path (path);

    cdir = getenv("MacaulayCdir");
    set_cdir (cdir);

    fp = topen ("Macaulay.path", "r");
    if (fp != NULL) {
        for (i=0, m=0; (c=fgetc(fp))!=EOF; ) {
            if (i == NN-1) {
                prerror ("; path truncated");
                break;
            }
            if (c == '\n') c = comma;
            if (c != comma) m = i;
            if (i==m || i==m+1) paths[i++] = c;
        }
        paths[m+1] = '\0';
        set_path (paths);
    }

    fp = topen ("Macaulay.cdir", "r");
    if (fp != NULL) {
        for (i=0; (c=fgetc(fp))!=EOF; ++i) {
            if (i == NN-1) {
                prerror ("; cdir truncated");
                break;
            }
            if (c == '\n') break;
            paths[i] = c;
        }
        paths[i] = '\0';
        set_cdir (paths);
    }
}

void path_cmd (int argc, char *argv[])
{
    char path[1024], *s;

    if (argc != 2) {
        printnew("path <path> (to replace)\n");
        printnew("path <:path> (to append)\n");
    }
    else
        set_path (argv[1]);

    /* display MclyPath */
    if (MclyPath != NULL) {
        if (argc != 2) print ("\n");
        strncpy (path, MclyPath, 1023);
        path[1023] = '\0';
        for (s=path; *s!='\0'; ++s)
            if (*s == comma) *s = '\n';
        print("%s\n", path);
    }
}

void cdir_cmd (int argc, char *argv[])
{
    if (argc != 2) {
        printnew("cdir <prefix>  (to set)\n");
        printnew("cdir .         (to clear)\n");
        if (MclyCdir != NULL)
            print("\n%s\n", MclyCdir);
    }
    else
        set_cdir (argv[1]);
}

static char *file_name (char *name, char *prefix, int prelen)
{
    int namelen;
    char *fullname;

    namelen = strlen(name);
    fullname = gimmy(prelen+namelen+2);
    if (prelen > 0) {
        strncpy(fullname, prefix, prelen);
        fullname[prelen] = colon;
        strncpy(fullname+prelen+1, name, namelen+1);
    }
    else {
        strncpy(fullname, name, namelen+1);
    }
    return fullname;
}

FILE *topen (char *fileName, char *mode)
{
    int flen, pos, once;
    FILE *fp, *fopen();
    char *path, *fname, *spos, *strchr(), *gimmy();

    fp = NULL;
    once = 0;
    if (MclyCdir != NULL && strchr(fileName, colon) == NULL) {
        pos = strlen(MclyCdir);
        fname = file_name (fileName, MclyCdir, pos);
        fp = fopen(fname, mode);
        ungimmy (fname);
        once = 1;
    }
    if (fp == NULL) {
        if (*mode == 'r' && MclyPath != NULL && *MclyPath != '\0'
                && strchr(fileName, colon) == NULL) {
            /* use MclyPath to search for file */
            flen = strlen(fileName);
            path = MclyPath;
            while (1) {
                if (*path == '\0') break;
                spos = strchr(path, comma);
                if (spos == NULL) pos = strlen(path);
                else pos = spos - path;
                fname = file_name (fileName, path, pos);
                fp = fopen(fname, mode);
                ungimmy(fname);
                once = 1;
                if (fp != NULL || path[pos] == '\0') break;
                path += pos+1;
            }
        }
        else if (!once) {
            fp = fopen(fileName, mode);
        }
    }

    return fp;
} /* mod 2Oct93 DB */

void sniff_prlevel (char *linebuf)
{
    static char prlev[] = "set prlevel 1";

    /* check for set or incr_set */
    if (strcmp(linebuf, prlev) == 0 ||
            (strncmp(linebuf, "incr", 4) == 0 && strcmp(linebuf+5, prlev) == 0)) {
        sniff = 1;
        ++prlevel;
    }
    else sniff = 0;
}

void unsniff_prlevel ()
{
    if (sniff == 1) --prlevel;
}

/* break_voice added: MES 3/29/88 */
void break_voice ()
{
    if (cur_voice > 0)
        drop_voice();
}

void drop_voice ()
{
    if (cur_voice > 0) {
        if (fd != stdin) fclose(fd);
        if (fj != NULL) fclose(fj);
        voice[level] = --cur_voice;
        fd = fstack[cur_voice];
        fj = jstack[cur_voice];
        in_dot_cmd_mode = dotflag[cur_voice];
        jf = jflag[cur_voice];
        inam = inames[cur_voice];
        nnam = nnames[cur_voice];
    }
    else {
        print("end of file ($) received, exiting Macaulay\n");
        exitMacaulay();
    }
}

int change_level ()
{
    int i;

    if (level >= NLEVELS) return 0;
    if (level < 0) {
        for (i=0; i<=cur_voice; ++i) {
            if (fstack[i] != stdin) fclose(fstack[i]);
            if (jstack[i] != NULL) fclose(jstack[i]);
        }
        old_level = -1;
        cur_voice = -1;
        return 1;
    }
    if (level < old_level) {
        for (i=voice[level]+1; i<=cur_voice; ++i) {
            if (fstack[i] != stdin) fclose(fstack[i]);
            if (jstack[i] != NULL) fclose(jstack[i]);
        }
        cur_voice = voice[level];
        fd = fstack[cur_voice];
        fj = jstack[cur_voice];
        in_dot_cmd_mode = dotflag[cur_voice];
        jf = jflag[cur_voice];
        inam = inames[cur_voice];
        nnam = nnames[cur_voice];
    }
    else if (level > old_level) {
        voice[level] = ++cur_voice;
        fd = fstack[cur_voice] = stdin;
        fj = jstack[cur_voice] = NULL;
        in_dot_cmd_mode = dotflag[cur_voice] = 0;
        jf = jflag[cur_voice] = 0;
        if (level == 0)
            inam = inames[cur_voice] = 0;
        else
            inam = inames[cur_voice] = inam + nnam;
        nnam = nnames[cur_voice] = 0;
    }
    old_level = level;
    return 1;
}

void prompt ()
{
    int i;

    if (!com)
        for (i=1; i<=nlines; i++)
            print("\n");
    for (i=0; i<=level; ++i) {
        if (dotflag[voice[i]]) {
            if (dotflag[voice[i]] == 1) print(".");
            else print("?"); /* debugging */
        }
        if (voice[i] > 0) print("%d", voice[i]);
        print("%% ");
    }
} /* 2/10/89 DB */

int get_chars (FILE *fd, char *a)
{
    /* read text into array a starting past last prompt, leading space */
    /* return value indicates if prompt was seen */
    int c, i = 0, flag = 0;
    int comment = 0, dollar = 0;

    while (c = getc(fd), !EOLN(c)) {
        if (c == '$') dollar = 1;
        if (c == ';') comment = 1;
        if (!comment && (c == PROMPT1 || c == PROMPT2)) {
            i = 0;
            flag = 1;
            continue;
        }
        if (i == 0 && (c == ' ' || c == '\t')) continue;
        if (i<NCHARS-1) a[i++] = c;
    }
    if ((c == EOF && i == 0) || (dollar == 1)) i=0, a[i++] = '$';
    a[i] = '\0';
    return flag;
}

void raw_line ()
{
    int i, new = 1, pflag;
    char userchars[NCHARS], *p;

beginning:
    do {
        if (new == 0) drop_voice();
        new = 0;
        if (cmd && fd == stdin && !remnull) prompt();
        pflag = get_chars(fd, linebuf);
        remnull = (cmd && linebuf[0] == '\0') ? 1 : 0;
    } while (linebuf[0] == '$');  /* $ is our EOF char */
    sniff_prlevel(linebuf);
    if (fj != NULL && !remnull) fprint(fj, "%s\n", linebuf);
    subs_names();
    if (linebuf[0] == '?' && (linebuf[1] == '\0' || isspace(linebuf[1]))) {
        if (cmd) prompt();
        else prinput("?");
        get_chars(stdin, linebuf);
        subs_names();
    }
    else if (fd != stdin) {
        if (cmd && !remnull) {
            if (linebuf[0] != ';') prompt();
            else if (!com)
                for (i=1; i<=nlines; i++)
                    print("\n");
        }
        if (!remnull) print("%s", linebuf);
        if (jf && linebuf[0] != '\0' && linebuf[0] != ';') {
            print(" ");
            get_chars(stdin, userchars);
            for (p=userchars; p[1]!=NULL; ++p);
            if (*p == '$') goto beginning;
            else if (*p == '&') debug_voice();
            else if (*p == '!') jf = jflag[cur_voice] = 0;
        }
        else if (!remnull) print("\n");
    }
    if (fd == stdin) {
        if (echo > 0)
        {
            if (!remnull) print("%s\n", linebuf);
        }
        else
            monprint("%s\n", linebuf);
        /* put linebuf in monitor file, if any */
    }
    unsniff_prlevel();
} /* 2/10/89 DB */

void get_line (int *pargc, char ***pargv)
{
    int i;
    char *s;

    raw_line();
    for (i=0, s=linebuf; i<NARGS; ) {
        while (!VISIBLE(*s) && *s != '\0') ++s;
        if (*s == '\0' || *s == ';') break;
        argv[i++] = s++;
        while (VISIBLE(*s)) ++s;
        if (*s == '\0') break;
        *s++ = '\0';
    }
    *pargc = i;
    *pargv = argv;
}

int open_journal (char *s)
{
    FILE *newfj, *topen();

    if (voice[level] == NVOICES-1) {
        prerror("; >>%s ignored, out of input voices\n", s);
        return 0;
    }
    newfj = topen(s, "w");
    if (newfj == NULL) {
        prerror("; >>%s ignored, error in opening file\n", s);
        return 0;
    }
    voice[level] = ++cur_voice;
    fd = fstack[cur_voice] = stdin;
    fj = jstack[cur_voice] = newfj;
    in_dot_cmd_mode = dotflag[cur_voice] = 0;
    jf = jflag[cur_voice] = 0;
    inam = inames[cur_voice] = inam + nnam;
    nnam = nnames[cur_voice] = 0;
    return 1;
}

int debug_voice ()
{
    if (voice[level] == NVOICES-1) {
        print("< ignored, out of input voices\n");
        return 0;
    }
    voice[level] = ++cur_voice;
    fd = fstack[cur_voice] = stdin;
    fj = jstack[cur_voice] = NULL;
    in_dot_cmd_mode = dotflag[cur_voice] = 0;
    jf = jflag[cur_voice] = 0;
    inam = inames[cur_voice] = inam + nnam;
    nnam = nnames[cur_voice] = 0;
    return 1;
}

int open_tour (char *s)
{
    FILE *newfd, *topen();
    int newjf;

    if (voice[level] == NVOICES-1) {
        prerror("; <%s ignored, out of input voices\n", s);
        return 0;
    }
    newjf = (*s == '<') ? 1 : 0;
    newjf = jf ? 1 : newjf;
    if (*s == '<') ++s;
    newfd = topen(s, "r");
    if (newfd == NULL) {
        prerror("; <%s ignored, error in opening file\n", s);
        return 0;
    }
    voice[level] = ++cur_voice;
    fd = fstack[cur_voice] = newfd;
    fj = jstack[cur_voice] = NULL;
    in_dot_cmd_mode = dotflag[cur_voice] = 0;
    jf = jflag[cur_voice] = newjf;
    inam = inames[cur_voice] = inam + nnam;
    nnam = nnames[cur_voice] = 0;
    return 1;
}

void set_names (int ac, char **av)
{
    int i, len;

    if (cur_voice == 0)
        top = 0;
    else {
        top = inames[cur_voice-1] + nnames[cur_voice-1];
        if (top > 0)
            top = indnames[top-1]
                  + strlen(namespace+indnames[top-1]) + 1;
    }
    for (i=1; i<ac; ++i) {
        len = strlen(av[i]);
        if (top+len+1 >= VNAMES || inam+nnam >= NNAMES) {
            print("Out of space; ignoring %s\n", av[i]);
            break;
        }
        indnames[inam+nnam++] = top;
        strcpy(namespace+top, av[i]);
        top += len + 1;
    }
    nnames[cur_voice] = nnam;
}

void subs_names ()
{
    int len, i, iline, two;
    char *p, *q, *r, *s, parm[NCHARS], at[5];

    for (p=linebuf; *p!='\0'; ++p) {
        if (
            (*p == '@')
            && !(p-linebuf >= 3 && p[-3] == '@' && isdigit(p[-2]) && isdigit(p[-1]))
            && !(strlen(p) >= 3 && p[3] == '@' && isdigit(p[2]) && isdigit(p[1]))
        ) {
            len = 4;
            iline = strlen(linebuf);
            if (iline + len - 1 >= NCHARS) {
                print("out of space; ");
                print("no substitution made for @\n");
                return;
            }
            for (r=linebuf+iline, s=r+len-1; r!=p; --r, --s) *s = *r;
            if (fd == stdin && fj == NULL) {
                if (voice[level] > 0)
                    sprintf(at, "@%02d@", voice[level]-1);
                else
                    sprintf(at, "@%02d@", voice[level]+1);
            }
            else /* normal case */
                sprintf(at, "@%02d@", voice[level]);
            q = at;
            for (; *q!='\0'; ++p, ++q) *p = *q;
            --p;
        }
    }

#define ZIP -31991 /* since parseInt() returns 0 on error */
    for (p=linebuf; *p!='\0'; ++p) {
        if (*p == '#') {
            if (isdigit(p[1])) {
                i = p[1] - '0';
                if (i == 0) i = ZIP;
                two = 2;
            }
            else if (p[1] == '(') {
                for (
                    i=0, q=p+2, r=parm;
                    (*q != '\0') && (i>0 || *q!=')');
                    ++q, ++r)
                {
                    if (*q == '(') ++i;
                    if (*q == ')') --i;
                    *r = *q;
                }
                if (*q == '\0') --q;
                *r = '\0';
                r = parm;
                i = parseInt(&r);
                two = q - p + 1;
            }
            else {
                r = p+1;
                i = parseInt(&r);
                two = r - p;
            }

            if (i == ZIP) {
                /* return # of args */
                sprintf(parm, "%d", nnam);
                q = parm;
            }
            else if (i > nnam) {
                parm[0] = '\0';
                q = parm; /* translate above-bounds ref to empty string */
            }
            else if (i < 1) {
                continue; /* leave offending ref intact */
            }
            else q = namespace + indnames[inam+i-1];

            len = strlen(q);
            if (len < two) {
                for (r=p+two, s=p+len; *s = *r, *r!='\0'; ++r, ++s);
            }
            else if (len > two) {
                iline = strlen(linebuf);
                if (iline + len - two >= NCHARS) {
                    print("out of space; ");
                    print("no substitution made for #%d\n", i);
                    return;
                }
                for (r=linebuf+iline, s=r+len-two; r!=p; --r, --s) *s = *r;
            }
            for (; *q!='\0'; ++p, ++q) *p = *q;
            --p;
        }
    }
} /* 5/18/89 DB, 8/16/93 DB */

void get_cmd_line (int *pargc, char ***pargv, char **poutfile)
{
    int i, ac;
    char *s, **av;
    int go_again;

    cmd = 1;
    *poutfile = NULL;
    do {
        get_line(&ac, &av);

        /* ignor label: arguments */
        if (ac > 0 && av[0][strlen(av[0])-1] == ':') {
            --ac;
            ++av;
        }

        com = linebuf[0] == ';' ? 1 : 0;
        if (ac == 0) continue;
        if (av[0][0] == '>' && av[0][1] == '>') {
            if (open_journal(av[0]+2)) set_names(ac, av);
            ac = 0;
            continue;
        }

        go_again = 0;
        for (i=0; i<ac; ++i) {
            if (av[i][0] == '<') {
                if(open_tour(av[i]+1)) set_names(ac-i, av+i);
                ac = i;
                go_again = 1;
                break;
            }
            if (av[i][0] == '>' && av[i][1] == '>') {
                if (open_journal(av[i]+2)) set_names(ac-i, av+i);
                ac = i;
                go_again = 1;
                break;
            }
        }
        if (go_again) continue; /* added MES 1/19/88, a crock? */
        /* of what, Mike? DB */
        s = av[ac-1];
        if (*s == '>') {
            --ac;
            *poutfile = ++s;
        }
    } while (ac == 0);
    cmd = 0;

    *pargc = ac;
    *pargv = av;
} /* 2/26/89 DB */

void branch (char *label)
{
    /* reset current file to beginning of line which starts "label:" */
    int len;
    char buf[NCHARS], *p;

    len = strlen(label);
    fseek(fd, 0L, 0);
    while (1) {
        if (fgets(buf, NCHARS-1, fd) == NULL) break;
        for (p=buf; isspace(*p); ++p);
        if (p[len] == ':' && strncmp(label, p, len) == 0) {
            len = strlen(buf);
            /* fseek( , , 1) is possible portability problem */
            /* kill next line and only label empty lines if trouble */
            fseek(fd, (long) -len, 1);
            break;
        }
    }
} /* 5/18/89 DB 4/6 */

void if_cmd (int argc, char *argv[])
{
    int test;

    if (argc < 3 || argc > 4) {
        printnew("if <integer> <label1> [<label2>]\n");
        printnew("if integer != 0 then jump to label1: [else jump to label2: ]\n");
    }
    else if (fd == stdin)
        printnew("if statements only take effect inside scripts\n");
    else {
        test = getInt(argv[1]);
        if (test != 0)
            branch(argv[2]);
        else if (argc == 4)
            branch(argv[3]);
    }
} /* new 2/26/89 DB */

void jump_cmd (int argc, char *argv[])
{
    if (argc != 2) {
        printnew("jump <label>\n");
        printnew("jump to label: in current input script\n");
    }
    else if (fd == stdin)
        printnew("jump statements only take effect inside scripts\n");
    else
        branch(argv[1]);
} /* new 2/26/89 DB */
