// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <assert.h>
#include <ctype.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shared.h"
#include "dot_fns.h"

// stash type should be defined in stash.h

#include "cmdnames.h"
#include "shell.h"
#include "vars.h"
#include "stash.h"
#include "monitor.h"
#include "cmds.h"
#include "input.h"
#include "gm_cmds.h"

constexpr int PMAX = 4;
constexpr int SMAX = 256;
constexpr int IDSIZE = 256; // Max identifier size
constexpr int VINT = 1;     // Variable type for integers

int in_dot_cmd_mode = 0;

typedef enum kinds
{
    end,
    ident,
    number,
    oper,
    brace,
    space,
    error
} kind;

static const char *kind_str[] = {"end", "ident", "number", "oper", "brace", "space", "error"};

// Forward declare item type
typedef struct item_struct item;

typedef struct
{
    const char *tok;
    const char *left, *right;
    void (*handler)(item *);
    int len;
    char lmem[PMAX + 1], rmem[PMAX + 1];
} tokentry;

// Forward declarations for handler functions
static void assign(item *p);
static void plus(item *p);
static void minus(item *p);
static void times(item *p);
static void concat(item *p);
static void transpose(item *p);
static void lparen(item *p);
static void rparen(item *p);
static void save(item *p);
static void command(item *p);
static void trace(item *p);
static void stop(item *p);
static void parse(item *p);

static tokentry tokens[] = {
    {"=", "z", "c", assign, 0, "", ""},     {"+", "m", "n", plus, 0, "", ""},
    {"-", "m", "n", minus, 0, "", ""},      {"*", "p", "q", times, 0, "", ""},
    {",", "r", "s", concat, 0, "", ""},     {"\\", "z", "x", transpose, 0, "", ""},
    {"(", "y", "b", lparen, 0, "", ""},     {")", "b", "y", rparen, 0, "", ""},
    {"`", "a", "c", save, 0, "", ""},       {".cmd", "z", "c", command, 0, "", ""},
    {".trace", "z", "x", trace, 0, "", ""}, {".stop", "z", "x", stop, 0, "", ""},
    {".parse", "z", "x", parse, 0, "", ""}, {"$", "a", "a", NULL, 0, "", ""},
    {NULL, NULL, NULL, NULL, 0, "", ""}};

static tokentry *end_entry, *cmd_entry, *quo_entry, *rpar_entry;

static tokentry endtoken = {"$", "", "", NULL, 0, "", ""};

struct item_struct
{
    kind what;
    const char *tok;
    cmd_rec *cmd;
    tokentry *entry;
    item *prev, *next;
};

typedef struct dot_frame_struct
{
    jmp_buf jmp_on_error;
    char *string_memory, *string_mem_end;
    int gensym_int;
    int stopping;
    item *item_list;
    struct dot_frame_struct *frame;
} dot_frame;

static dot_frame *frame = NULL;
static struct stash *item_stash = NULL;
static int gensym_int = 0;
static int tracing = 0;

// **********************************************

static void print_tokenized(item *q)
{
    // for debugging dot functions
    item *p;
    char s[IDSIZE + 1];
    int i;

    if (flushnum)
    {
        print("\n");
        flushnum = 0; // reset verbose output
    }
    for (i = 2, p = frame->item_list; i > 0; p = p->next)
    {
        if (p->what == end)
            --i;
        print("%s ", p->tok);
    }
    print("\n");
    if (q != NULL)
    {
        for (i = 2, p = frame->item_list; i > 0; p = p->next)
        {
            if (p->what == end)
                --i;
            if (p == q)
                sprintf(s, "^%*s", (int)strlen(p->tok), "");
            else
                sprintf(s, "%*s ", (int)strlen(p->tok), "");
            print("%s", s);
        }
        print("\n");
    }
}

static void dot_error(item *p)
{
    print_tokenized(p);
    frame->stopping = -1; // don't kill temp vars
    longjmp(frame->jmp_on_error, 1);
}

static void dot_arg_error(item *op, item *arg)
{
    prerror("; invalid argument to %s\n", op->tok);
    dot_error(arg);
}

__attribute__((unused)) static void print_token_list(void)
{
    // for debugging dot functions
    item *p;
    int i;

    print("\n");
    for (i = 0, p = frame->item_list; i == 0 || p->what != end; ++i, p = p->next)
    {
        print("{ %s, %s", kind_str[p->what], p->tok);
        if (p->entry)
        {
            print(", {%s, %s, %s}", p->entry->tok, p->entry->left, p->entry->right);
        }
        print(" }\n");
    }
    print("\n");
}

// **********************************************

void init_dot(void)
{
    // call at startup or reset from shell.c

    item_stash = NULL;
}

static int is_op(const char *s, tokentry **entry)
{
    // return length of longest recognized oper, and set entry
    tokentry *p;
    int len = 0;

    for (p = tokens; p->tok != NULL; ++p)
        if (p->len > len && strncmp(s, p->tok, (size_t)p->len) == 0)
        {
            *entry = p;
            len = p->len;
        }
    return len;
}

static void private_init(void)
{
    // call from dot_cmd
    tokentry *p;

    frame->gensym_int = gensym_int;
    frame->stopping = -2;
    if (!item_stash)
    {
        item_stash = (struct stash *)open_stash(sizeof(item), "dot items");
        for (p = tokens; p->tok != NULL; ++p)
            p->len = (int)strlen(p->tok);
        is_op("$", &end_entry);
        is_op(".cmd", &cmd_entry);
        is_op("`", &quo_entry);
        is_op("(", &rpar_entry);
    }
}

static char *make_string(const char *s, int len)
{
    char *t;

    if (len + 1 > frame->string_mem_end - frame->string_memory)
    {
        prerror("; out of memory for strings\n");
        longjmp(frame->jmp_on_error, 1);
    }
    t = frame->string_memory;
    frame->string_memory += len + 1;
    strncpy(t, s, (size_t)len);
    t[len] = '\0';
    return t;
}

__attribute__((unused)) static char *read_src(int argc, char *argv[], int n)
{
    static int cur_len, cur_word;
    static char *cur_string;

    if (n == 0)
    {
        // reset at beginning of source line
        cur_word = in_dot_cmd_mode ? 0 : 1;
    }
    else
    {
        // advance n chars or to next arg and return pointer
        if (n < cur_len)
        {
            cur_len -= n;
            return cur_string += n;
        }
        else
        {
            ++cur_word;
        }
    }
    if (argc <= cur_word)
        return NULL;
    cur_string = argv[cur_word];
    cur_len = (int)strlen(cur_string);
    return cur_string;
}

static int space_char(int c)
{
    return (isspace(c));
}

static int digit_char(int c)
{
    return (isdigit(c));
}

static int cmd_char1(int c)
{
    return (isalpha(c));
}

static int cmd_char2(int c)
{
    return (isalpha(c) || c == '_');
}

static int ident_char1(int c)
{
    return (isalpha(c) || c == '@');
}

static int ident_char2(int c)
{
    return (isalnum(c) || c == '_' || c == '@' || c == '\'' || c == '.');
}

static int is_token(const char *s, int (*char1)(int), int (*char2)(int))
{
    // return length of recognized token

    int i = 0;
    if (char1(*s))
        while (++i, *++s != '\0')
            if (!char2(*s))
                break;
    return i;
}

static int is_cmd(const char *s, char **t, cmd_rec **cmd)
{
    // return length of recognized command
    // set t to string, *cmd to &cmd_rec
    int len, i;
    char name[SMAX];

    len = is_token(s, cmd_char1, cmd_char2);
    if (len == 0)
        return 0;
    strncpy(name, s, (size_t)len);
    name[len] = '\0';
    i = cmd_lookup(name, 4);
    if (i >= 0)
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
        *t = (char *)cmd_list[i].name; // C23: OK - not modified
#pragma GCC diagnostic pop
        *cmd = cmd_list + i;
        return len;
    }
    else
        return 0;
}

static int is_quote(char *s, char **t)
{
    // return length of recognized quoted string
    // set t to string
    int i = 0;
    if (*s == '"')
    {
        *t = s + 1;
        while (++i, *++s != '\0')
            if (*s == '"')
                break;
        *s = '\0';
        ++i;
    }
    return i;
}

static int is_brace(const char *s, char **t)
{
    // return length of recognized braced text
    // set t to string
    int i = 0;
    if (*s == '{')
    {
        int d = 1;
        const char *start = s;
        while (++i, *++s != '\0')
        {
            if (*s == '{')
                ++d;
            if (*s == '}')
                --d;
            if (d == 0)
                break;
        }
        ++i;
        *t = make_string(start, i);
    }
    return i;
}

static char *gensym_ident(void)
{
    char st[16];

    sprintf(st, "@00@%d", ++gensym_int);
    return make_string(st, (int)strlen(st));
}

static void tokenize_src(int argc, char *argv[])
{
    char *s, *t, *src;
    item *p, *q;
    tokentry *entry;
    kind what;
    int len, n;
    cmd_rec *cmd;

    p = q = frame->item_list = (item *)(void *)get_slug(item_stash);
    p->what = end;
    p->tok = end_entry->tok;
    p->entry = end_entry;
    p->prev = NULL;

    // reconstruct single string from arguments
    n = in_dot_cmd_mode ? 0 : 1;
    s = src = argv[n];
    for (++n; n < argc; ++n)
    {
        do
        {
            ++s;
        } while (*s != '\0');
        *s = ' ';
    }

    s = src;
    what = space;
    while (*s != '\0')
    {
        t = NULL;
        cmd = NULL;
        entry = NULL;
        if ((len = is_token(s, space_char, space_char)))
            what = space;
        else if ((len = is_cmd(s, &t, &cmd)))
        {
            what = oper;
            entry = cmd_entry;
        }
        else if ((len = is_quote(s, &t)))
            what = ident;
        else if ((len = is_brace(s, &t)))
            what = brace;
        else if ((len = is_token(s, ident_char1, ident_char2)))
            what = ident;
        else if (*s == '_')
        {
            len = 1;
            what = ident;
            t = gensym_ident();
        }
        else if ((len = is_token(s, digit_char, digit_char)))
            what = number;
        else if (*s == '-' && (what == space || (what == oper && p->entry != rpar_entry)) &&
                 (len = is_token(s + 1, digit_char, digit_char)))
        {
            ++len;
            what = number;
        }
        else if ((len = is_op(s, &entry)))
            what = oper;
        else
        {
            what = error;
            len = (int)strlen(s);
        }

        if (what != space)
        {
            p = (item *)(void *)get_slug(item_stash);
            p->prev = q;
            q->next = p;
            q = p;
            p->what = what;
            p->tok = t ? t : make_string(s, len);
            p->cmd = cmd;
            p->entry = entry;
            if (what == error)
            {
                p->next = frame->item_list;
                frame->item_list->prev = p;
                prerror("; unrecognized token\n");
                dot_error(p);
            }
        }
        s += len;
    }
    p = frame->item_list;
    p->prev = q;
    q->next = p;
}

static item *prev_op(item *p)
{
    // return next oper before p, or end marker
    for (p = p->prev; p->what != end; p = p->prev)
        if (p->what == oper)
            break;
    return p;
}

__attribute__((unused)) static void debug_parse(item *p, item *q)
{
    if (p->entry)
        print("{%s, %s, %s} ", p->entry->left, p->entry->tok, p->entry->right);
    else
        print("{ NULL } ");
    if (q->entry)
        print("{%s, %s, %s}\n", q->entry->left, q->entry->tok, q->entry->right);
    else
        print("{ NULL } ");
}

static void parse_src(void)
{
    item *p, *q;
    int debug;

    while (1)
    {
        q = frame->item_list;
        do
        {
            p = q;
            q = prev_op(p);
        } while ((debug = strcmp(q->entry->right, p->entry->left) > 0));
        if (p->what == end)
        {
            if (tracing && p->next->what != end)
                print_tokenized(NULL);
            return;
        }
        if (tracing)
            print_tokenized(p);
        assert(p->entry != NULL && p->entry->handler != NULL);
        (p->entry->handler)(p);
        if (frame->stopping > -1 && --(frame->stopping) == -1)
        {
            print_tokenized(NULL);
            return;
        }
    }
}

static item *delete_item(item *p)
{
    // remove p, returning next item
    item *q;
    variable *v;

    p->prev->next = p->next;
    p->next->prev = p->prev;
    q = p->next;
    if (q == p)
        q = NULL;
    if (p->what == ident && strncmp(p->tok, "@00@", 4) == 0 && frame->stopping == -2)
    {
        v = find_var(p->tok);
        if (v != NULL)
            rem_var(v);
    }
    free_slug(item_stash, (struct slug *)p);
    return q;
}

static void delete_item_list(void)
{
    item *p;

    if (frame->item_list != NULL)
    {
        for (p = frame->item_list; p != NULL; p = delete_item(p))
            ;
        frame->item_list = NULL;
    }
}

static void mop_up(void)
{
    delete_item_list();
    gensym_int = frame->gensym_int;
}

int dot_cmd(int argc, char *argv[])
{
    dot_frame frame_rec;
    char mem[SMAX];

    if (argc == 1 && strcmp(argv[0], ".") == 0)
    {
        in_dot_cmd_mode = dotflag[cur_voice] = !in_dot_cmd_mode;
        return 0;
    }

    frame_rec.frame = frame;
    frame = &frame_rec;
    frame->string_memory = mem;
    frame->string_mem_end = mem + SMAX;
    private_init();

    if (setjmp(frame->jmp_on_error))
    {
        mop_up();
        frame = frame_rec.frame;
        return 0;
    }

    tokenize_src(argc, argv);
    // print_token_list();
    parse_src();
    mop_up();
    frame = frame_rec.frame;
    return 0;
}

// **********************************************

static void insert_item(item *p, item *q)
{
    // insert q before p
    p->prev->next = q;
    q->prev = p->prev;
    q->next = p;
    p->prev = q;
}

static void patch_items(item *p, int left, int right, item *q)
{
    int i;
    int total_items;

    // remove p from item_list
    // remove left # items from left of p
    // remove right # items from right of p
    for (i = 0; i < left; ++i)
        p = p->prev;
    // Calculate total items to avoid overflow warning
    total_items = left + right + 1;
    for (i = 0; i < total_items; ++i)
        p = delete_item(p);

    // replace with q if not NULL
    if (q != NULL)
        insert_item(p, q);
}

// **********************************************

static item *new_item(kind what, const char *tok)
{
    item *t;

    t = (item *)(void *)get_slug(item_stash);
    t->what = what;
    t->tok = tok;
    t->entry = NULL;
    return t;
}

static int is_save(item *p)
{
    return (p->what == oper && p->tok[0] == '`');
}

static char *insert_quoted_var(item *p)
{
    // insert `_ before p, return tok for _
    char *s;
    item *t;

    t = new_item(oper, "`");
    t->entry = quo_entry;
    insert_item(p, t);

    s = gensym_ident();
    t = new_item(ident, s);
    insert_item(p, t);
    return s;
}

static void command(item *p)
{
    char *argv[16];
    int i, argc;
    item *q;

    if (p->cmd == NULL)
    {
        prerror("; cannot use .cmd as a command\n");
        dot_error(p);
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    argv[0] = (char *)p->tok; // C23: OK - commands expect char* argv
    for (argc = 1, q = p->next; argc < 16; ++argc, q = q->next)
    {
        while (is_save(q))
            q = q->next;
        if (q->what != ident && q->what != number)
            break;
        argv[argc] = (char *)q->tok; // C23: OK - commands expect char* argv
    }
#pragma GCC diagnostic pop
    if (p->prev->what != end)
    {
        int required_args = 1 + p->cmd->args;
        while (argc < required_args)
            argv[argc++] = insert_quoted_var(q);
    }

    cmd_error = 0;
    (*p->cmd->proc)(argc, argv);
    if (cmd_error)
        dot_error(p);

    for (q = p->next, i = 1; i < argc; ++i)
    {
        if (is_save(q))
        {
            q = delete_item(q);
            while (is_save(q))
                q = q->next;
            q = q->next;
        }
        else
            q = delete_item(q);
    }
    patch_items(p, 0, 0, NULL);
}

static void save(item *p)
{
    patch_items(p, 0, 0, NULL);
}

static void trace(item *p)
{
    tracing = !tracing;
    print("; tracing %s\n", tracing ? "on" : "off");
    patch_items(p, 0, 0, NULL);
}

static void stop(item *p)
{
    if (p->next->what != number)
        dot_arg_error(p, p->next);
    sscanf(p->next->tok, "%d", &(frame->stopping));
    patch_items(p, 0, 1, NULL);
}

static void parse(item *p)
{
    item *qop, *ql, *qr;
    tokentry *t;
    int n;

    if ((qop = p->prev)->what == oper && (ql = p->next)->what == ident &&
        (qr = ql->next)->what == ident)
    {
        t = qop->entry;

        n = (int)strlen(ql->tok);
        n = n > PMAX ? PMAX : n;
        strncpy(t->lmem, ql->tok, (size_t)n);
        t->lmem[n] = '\0';
        t->left = t->lmem;

        n = (int)strlen(qr->tok);
        n = n > PMAX ? PMAX : n;
        strncpy(t->rmem, qr->tok, (size_t)n);
        t->rmem[n] = '\0'; // Fixed typo: was t->lmem[n]
        t->right = t->rmem;

        patch_items(p, 1, 2, NULL);
    }
    else
    {
        for (t = tokens; t->tok != NULL; ++t)
            print("%16s  %4s %4s\n", t->tok, t->left, t->right);
        patch_items(p, 0, 0, NULL);
    }
}

static int is_lvalue(item *p)
{
    return (p->what == ident);
}

static int is_rvalue(item *p)
{
    return (p->what == ident || p->what == number);
}

static void check_variable(variable *v, item *p)
{
    if (v == NULL)
        dot_error(p);
}

static int int_or_mod(item *op, item *p)
{
    // return 1 for int, 0 for mod, else throw error
    variable *v;

    if (p->what == number)
        return 1;
    else
    {
        if (p->what != ident)
            dot_arg_error(op, p);
        v = find_var(p->tok);
        check_variable(v, p);
        if (v->type == VINT)
            return 1;
        else
            return 0;
    }
}

// Wrapper functions for commands that return int
static void int_cmd_wrapper(int argc, char *argv[])
{
    (void)int_cmd(argc, argv);
}

static void copy_cmd_wrapper(int argc, char *argv[])
{
    (void)copy_cmd(argc, argv);
}

static void do_cmd2(item *p, void (*cmd)(int, char *[]), const char *s1, const char *s2)
{
    char *argv[3];

    cmd_error = 0;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    argv[0] = (char *)""; // C23: OK - not modified
    argv[1] = (char *)s1; // C23: OK - commands expect char* argv
    argv[2] = (char *)s2; // C23: OK - commands expect char* argv
#pragma GCC diagnostic pop
    (*cmd)(3, argv);
    if (cmd_error)
        dot_error(p);
}

static void do_cmd3(item *p, void (*cmd)(int, char *[]), const char *s1, const char *s2,
                    const char *s3)
{
    char *argv[4];

    cmd_error = 0;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    argv[0] = (char *)""; // C23: OK - not modified
    argv[1] = (char *)s1; // C23: OK - commands expect char* argv
    argv[2] = (char *)s2; // C23: OK - commands expect char* argv
    argv[3] = (char *)s3; // C23: OK - commands expect char* argv
#pragma GCC diagnostic pop
    (*cmd)(4, argv);
    if (cmd_error)
        dot_error(p);
}

static void assign(item *p)
{
    variable *vr = NULL, *vq = NULL;
    int i, j, intflag;
    item *q, *r;

    // q = leftmost identifier, r = leftmost value, i = count
    for (i = 0, q = p, r = p->next;; ++i, q = q->prev, r = r->next)
    {
        if (!is_lvalue(q->prev))
            break;
        if (!is_rvalue(r))
            break;
    }
    if (i == 0)
    {
        prerror("; no arguments to %s\n", p->tok);
        dot_error(p);
    }
    r = p->next;

    // make assignment for each pair
    for (j = 1; j <= i; ++j, q = q->next, r = r->next)
    {
        // identify rvalue as suitable for int_cmd or copy_cmd
        intflag = int_or_mod(p, r);

        // apply suitable command
        if (intflag)
            do_cmd2(r, int_cmd_wrapper, q->tok, r->tok);
        else
            do_cmd2(r, copy_cmd_wrapper, r->tok, q->tok);
    }
    patch_items(p, 0, i, NULL);
}

static void binary_op(item *p, void (*mod_cmd)(int, char *[]), const char *int_op)
{
    item *q, *r, *t;
    char *s, arg[SMAX];
    int qflag, rflag;

    r = p->next;
    q = p->prev;
    rflag = int_or_mod(p, r);
    qflag = int_or_mod(p, q);
    if (qflag != rflag)
        dot_arg_error(p, q);
    s = gensym_ident();

    // apply suitable command
    cmd_error = 0;
    if (qflag)
    {
        if (int_op == NULL)
            dot_arg_error(p, q);
        sprintf(arg, "%s%s%s", q->tok, int_op, r->tok);
        do_cmd2(p, int_cmd_wrapper, s, arg);
    }
    else
    {
        if (mod_cmd == NULL)
            dot_arg_error(p, q);
        do_cmd3(p, mod_cmd, q->tok, r->tok, s);
    }
    if (cmd_error)
        dot_error(p);

    t = new_item(ident, s);
    patch_items(p, 1, 1, t);
}

static void plus(item *p)
{
    binary_op(p, add_cmd, "+");
}

static void minus(item *p)
{
    binary_op(p, sub_cmd, "-");
}

static void times(item *p)
{
    binary_op(p, mult_cmd, "*");
}

static void concat(item *p)
{
    item *q, *r, *t;
    char *s;

    r = p->next;
    q = p->prev;
    s = gensym_ident();

    do_cmd2(p, copy_cmd_wrapper, q->tok, s);
    do_cmd2(p, concat_cmd, s, r->tok);

    t = new_item(ident, s);
    patch_items(p, 1, 1, t);
}

static void transpose(item *p)
{
    item *r, *t;
    char *s;

    r = p->next;
    s = gensym_ident();

    do_cmd2(p, trans_cmd, r->tok, s);

    t = new_item(ident, s);
    patch_items(p, 0, 1, t);
}

static void lparen(item *p)
{
    item *q;

    for (q = p->next; q->what != end; q = q->next)
        if (q->what == oper && q->entry->handler == rparen)
            break;
    if (q->what != end)
        patch_items(q, 0, 0, NULL);
    patch_items(p, 0, 0, NULL);
}

static void rparen(item *p)
{
    item *q;

    for (q = p->prev; q->what != end; q = q->prev)
        if (q->what == oper && q->entry->handler == lparen)
            break;
    if (q->what != end)
        patch_items(q, 0, 0, NULL);
    patch_items(p, 0, 0, NULL);
}
