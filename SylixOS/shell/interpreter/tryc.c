/*
 * tryc.c
 *
 *  Created on: May 17, 2021
 *      Author: 25412
 */

#include <stdio.h>
#include <float.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "shellcall.h"

#define POOLSIZE  (256 * 1024)  /* arbitrary size */
#define SYMTABSIZE (1024*8)     /* size of the symbol table stack */
#define MAXNAMESIZE 32          /* size of variable/function name */
#define RETURNFLAG DBL_MAX             /* indicate a return statement has occured in a function */

#define MAX_CMD_LENTH 1024
#define CMD_OUT_BUF_LENGTH 1024

#ifndef bool
#define bool int
#define false 0
#define true 1

#endif

static bool break_flag = false;
static bool continue_flag = false;

/* this structure represent a symbol store in a symbol table */
typedef struct symStruct {
    int type;                  /*the type of the symbol:  Num, Char, Str, Array, Func*/
    char name[MAXNAMESIZE];    /* record the symbol name */
    double value;              /* record the symbol value, or the length of string or array */
    union {
        char* funcp;            /* pointer to an array or an function */
        struct symStruct* list;
    } pointer;
    int levelNum;               /* indicate the declare nesting level */
} symbol;
static symbol symtab[SYMTABSIZE];
static int symPointer = 0;             /* the symbol stack pointer */
static int currentlevel = 0;           /* current nesting level */

static char* src, * old_src;           /* the text process currently */

enum {
    debug, run
};
static int compileState = run;          /* current interpre state */

static double return_val = 0;           /* used for reserve the return value of function */

/* tokens and classes (operators last and in precedence order) */
enum {
    Num = 128, Char, NamedStr, UnnamedStr, Array, Func,
    Else, If, Return, While, Print,Puts, Read, Continue, Break, Call,
    Assign, OR, AND, Equal, Sym, FuncSym, ArraySym, Void,
    Nequal, LessEqual, GreatEqual
};
static int token;                      /* current token type */
union tokenValue {
    union {
        symbol *ptr;
        char *str;
    };
    double val;                 /* token value, for Char or Num */
};
static union tokenValue token_val;

/*--------------- function declaration ---------------*/
static double function();
static double statement();
static int boolOR();
static int boolAND();
static int boolexp();
static double expression();
static double factor();
static double term();
static void match(int tk);
static void next();
static void release_symbol(symbol *psymbol);
static void match_multiple(int *tokens, int len);
static void index_error();
static void tryc_init();
/* -------------------  lexical analysis  ---------------------------------*/
/* get the next token of the input string */
static void next() {
    char* last_pos;

    while ((token = *src)) {
        ++src;
        if (token == '\n') {                /* a new line */
            if(compileState == debug)       /* if on debug mode, print the currnet process line */
                printf("%.*s",  (int)(src - old_src), old_src);
            old_src = src;
        }
        else if (token == '#') {            /* skip comments */
            while (*src != 0 && *src != '\n') {
                src++;
            }
        }
        else if ((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z') || (token == '_')) {
            last_pos = src - 1;             /* process symbols */
            char nameBuffer[MAXNAMESIZE];
            nameBuffer[0] = token;
            while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || (*src == '_')) {
                nameBuffer[src - last_pos] = *src;
                src++;
            }
            nameBuffer[src - last_pos] = 0;                 /* get symbol name */
            int i;
            for (i = symPointer-1; i >= 0; --i) {           /* search symbol in symbol table */
                if (strcmp(nameBuffer, symtab[i].name) == 0) {      /* if find symbol: return the token according to symbol type */
                    if (symtab[i].type == Num || symtab[i].type == Char) {
                        token_val.ptr = &symtab[i];
                        token = Sym;
                    }
                    else if (symtab[i].type == FuncSym) {
                        token_val.ptr = &symtab[i];
                        token = symtab[i].type;
                    }
                    else if (symtab[i].type == ArraySym) {
                        token_val.ptr = &symtab[i];
                        token = symtab[i].type;
                    }
                    else if (symtab[i].type == NamedStr) {
                        token_val.ptr = &symtab[i];
                        token = symtab[i].type;
                    }
                    else {
                        if (symtab[i].type == Void) {
                            token = Sym;
                            token_val.ptr = &symtab[i];
                        }
                        else token = symtab[i].type;
                    }
                    return;
                }
            }
            strcpy(symtab[symPointer].name, nameBuffer);        /* if symbol not found, create a new one */
            symtab[symPointer].levelNum = currentlevel;
            symtab[symPointer].type = Void;
            token_val.ptr = &symtab[symPointer];
            symPointer++;
            token = Sym;
            return;
        }
        else if (token >= '0' && token <= '9') {        /* process numbers */
            token_val.val = (double)token - '0';
            while (*src >= '0' && *src <= '9') {
                token_val.val = token_val.val * 10.0 + *src++ - '0';
            }
            if (*src == '.') {
                src++;
                int countDig = 1;
                while (*src >= '0' && *src <= '9') {
                    token_val.val = token_val.val + ((double)(*src++) - '0')/(10.0 * countDig++);
                }
            }
            token = Num;
            return;
        }
        else if (token == '\'') {               /* parse char */
            token_val.val = *src++;
            token = Char;
            src++;
            return;
        }
        else if (token == '"' ) {               /* parse string */
            last_pos = src;
            int numCount = 0;
            while (*src != 0 && *src != token) {
                src++;
                numCount++;
            }
            if (*src) {
                *src = 0;
                token_val.str = malloc(sizeof(char) * numCount + 8);
                strcpy(token_val.str, last_pos);
                *src = token;
                src++;
            }
            token = UnnamedStr;
            return;
        }
        else if (token == '=') {            /* parse '==' and '=' */
            if (*src == '=') {
                src++;
                token = Equal;
            }
            return;
        }
        else if (token == '!') {               /* parse '!=' */
            if (*src == '=') {
                src++;
                token = Nequal;
            }
            return;
        }
        else if (token == '<') {               /* parse '<=',  or '<' */
            if (*src == '=') {
                src++;
                token = LessEqual;
            }
            return;
        }
        else if (token == '>') {                /* parse '>=',  or '>' */
            if (*src == '=') {
                src++;
                token = GreatEqual;
            }
            return;
        }
        else if (token == '|') {                /* parse  '||' */
            if (*src == '|') {
                src++;
                token = OR;
            }
            return;
        }
        else if (token == '&') {                /* parse  '&&' */
            if (*src == '&') {
                src++;
                token = AND;
            }
            return;
        }
        else if ( token == '*' || token == '/'  || token == ';' || token == ',' || token == '+' || token == '-' ||
            token == '(' || token == ')' || token == '{' || token == '}' ||  token == '[' || token == ']') {
            return;
        }
        else if (token == ' ' || token == '\t') {        }
        else {
            printf("unexpected token: %c\n", token);
        }
    }
}

static void match(int tk) {
    if (token == tk) {
        if (compileState == debug) {
            if(isprint(tk))
                printf("match: %c\n", tk );
            else
                printf("match: %d\n", tk);
        }
        next();
    }
    else {
        printf("line %.*s:expected token: %c\n", (int)(src - old_src), old_src,  tk);
        exit(-1);
    }
}

static void match_multiple(int *tokens, int len) {
    int i;
    for (i=0; i<len; ++i) {
        if (tokens[i] == token) {
            if (compileState == debug) {
                if(isprint(tokens[i]))
                    printf("match: %c\n", tokens[i]);
                else
                    printf("match: %d\n", tokens[i]);
            }
            next();
            return;
        }
    }
    printf("line %.*s:expected token: ", (int)(src - old_src), old_src);
    for (i=0; i<len; ++i) {
        printf("%c, ", tokens[i]);
    }
    puts(".");
    exit(-1);
}

/*--------------------------  grammatical analysis and run ----------------------------------*/

static double term() {
    double temp = factor();
    while (token == '*' || token == '/') {
        if (token == '*') {
            match('*');
            temp *= factor();
        }
        else {
            match('/');
            temp /= factor();
        }
    }
    return temp;
}

static double factor() {
    double temp = 0;
    if (token == '(') {
        match('(');
        temp = expression();
        match(')');
    }
    else if(token == Num ||  token == Char){
        temp = token_val.val;
        match(token);
    }
    else if (token == Sym) {
        temp = token_val.ptr->value;
        match(Sym);
    }
    else if (token == FuncSym) {
        return function();
    }
    else if (token == ArraySym || token == NamedStr) {
        symbol* ptr = token_val.ptr;
        int tokens[2] = {ArraySym, NamedStr};
        match_multiple(tokens, 2);
        match('[');
        int index = (int)expression();
        if (ptr->type == ArraySym) {
            if (index >= 0 && index < ptr->value) {
                temp = ptr->pointer.list[index].value;
            }
            else {
                index_error();
            }
        }
        /* TODO: strlen */
        else if (ptr->type == NamedStr) {
            if (index >= 0 && index <= strlen(ptr->pointer.funcp)) {
                temp = ptr->pointer.funcp[index];
            }
            else {
                index_error();
            }
        }
        match(']');
    }
    return temp;
}

static double expression() {
    double temp = term();
    while (token == '+' || token == '-') {
        if (token == '+') {
            match('+');
            temp += term();
        }
        else {
            match('-');
            temp -= term();
        }
    }
    return temp;
}

static int boolexp() {
    if (token == '(') {
        match('(');
        int result = boolOR();
        match(')');
        return result;
    }
    else if (token == '!') {
        match('!');
        return !boolexp();
    }
    double temp = expression();
    if (token == '>') {
        match('>');
        return temp > expression();
    }
    else if (token == '<') {
        match('<');
        return temp < expression();
    }
    else if (token == GreatEqual) {
        match(GreatEqual);
        return temp >= expression();
    }
    else if (token == LessEqual) {
        match(LessEqual);
        return temp <= expression();
    }
    else if (token == Equal) {
        match(Equal);
        return temp == expression();
    }
    return 0;
}

static int boolAND() {
    int val = boolexp();
    while (token == AND) {
        match(AND);
        if (val == 0)    return 0;         /* short cut */
        val = val & boolexp();
        if (val == 0) return 0;
    }
    return val;
}

static int boolOR() {
    int val = boolAND();
    while (token == OR) {
        match(OR);
        if (val == 1)    return 1;         /* short cut */
        val = val | boolAND();
    }
    return val;
}

static void skipStatments() {
    if(token == '{')
        token = *src++;
    int count = 0;
    while (token && !(token == '}' && count == 0)) {
        if (token == '}') count++;
        if (token == '{') count--;
        token = *src++;
    }
    match('}');
}

static double statement() {
    if (token == '{') {
        match('{');
        while (token != '}') {
            if (RETURNFLAG == statement())
                return RETURNFLAG;
            if (continue_flag || break_flag) {
                skipStatments();
                return 0;
            }
        }
        match('}');
    }
    else if (token == If) {
        match(If);
        match('(');
        int boolresult = boolOR();
        match(')');
        if (boolresult) {
            if (RETURNFLAG == statement())
                return RETURNFLAG;
        }
        else skipStatments();
        if (token == Else) {
            match(Else);
            if (!boolresult) {
                if (RETURNFLAG == statement())
                    return RETURNFLAG;
            }
            else skipStatments();
        }
    }
    else if (token == While) {
        match(While);
        char* whileStartPos = src;
        char* whileStartOldPos = old_src;
        int boolresult;
        do {
            src = whileStartPos;
            old_src = whileStartOldPos;
            token = '(';
            match('(');
            boolresult = boolOR();
            match(')');
            if (boolresult) {
                if (RETURNFLAG == statement())
                    return RETURNFLAG;
            }
            else skipStatments();
            assert((break_flag^continue_flag) || (!break_flag && !continue_flag));
            if (continue_flag) {
                continue_flag = false;
            }
            else if (break_flag) {
                break_flag = true;
                break;
            }
        }while (boolresult);
    }
    else if (token == Continue) {
        match(Continue);
        match(';');
        continue_flag = 1;
    }
    else if (token == Break) {
        match(Break);
        match(';');
        break_flag = 1;
    }
    else if (token == Sym || token == ArraySym || token == NamedStr) {
        symbol* s = token_val.ptr;
        int tktype = token;
        int index;
        match(tktype);
        if (tktype == ArraySym) {
            match('[');
            index = expression();
            match(']');
            match('=');
            if (index >= 0 && index < s->value) {
                release_symbol(&s->pointer.list[index]);
                s->pointer.list[index].value = expression();
            }
            else {
                index_error();
            }
        }
        else if (tktype == NamedStr && token == '[') {
            /* assign to a string element */
            match('[');
            index = expression();
            match(']');
            match('=');
            /* TODO: record the length of the string to avoid using strlen */
            if (index >= 0 && index <= strlen(s->pointer.funcp)) {
                int ch = (int)expression();
                if (ch >= 128 || ch < -128) {
                    puts("Warning: assign to a char with a value out of range.");
                }
                s->pointer.funcp[index] = (char)ch;
            }
            else {
                index_error();
            }
        }
        else {
            release_symbol(s);
            match('=');
            if (token == UnnamedStr) {
                s->pointer.funcp = token_val.str;
                s->type = NamedStr;
                match(UnnamedStr);
            }
            else if (token == Char) {
                s->value = token_val.val;
                s->type = Char;
                match(Char);
            }
            else {
                s->value = expression();
                s->type = Num;
            }
        }
        match(';');
    }
    else if (token == Array) {
        match(Array);
        symbol* s = token_val.ptr;
        match(Sym);
        match('(');
        int length = (int)expression();
        match(')');
        s->pointer.list = (struct symStruct*)malloc(sizeof(struct symStruct) * length + 1);
        int i;
        for (i = 0; i < length; ++i)
            s->pointer.list[i].type = Num;
        s->value = length;
        s->type = ArraySym;
        match(';');
    }
    else if (token == Func) {
        match(Func);
        symbol* s = token_val.ptr;
        s->type = FuncSym;
        match(Sym);
        s->pointer.funcp = src;
        s->value = token;
        skipStatments();
        match(';');
    }
    else if (token == Return) {
        match(Return);
        match('(');
        return_val = expression();
        match(')');
        match(';');
        return RETURNFLAG;
    }
    else if (token == Print || token == Read || token == Puts) {
        int func = token;
        double temp;
        match(func);
        match('(');
        int cnt;
        switch (func) {
        case Print:
            temp = expression();
            printf("%lf\n", temp);
            break;
        case Puts:
            if (token == UnnamedStr) {
                printf("%s\n", token_val.str);
                free(token_val.str);
            }
            else if (token == NamedStr) {
                printf("%s\n", token_val.ptr->pointer.funcp);
            }
            int tokens[] = {UnnamedStr, NamedStr};
            match_multiple(tokens, 2);
            break;
        case Read:
            cnt = scanf("%lf", &token_val.ptr->value);
            if (cnt != 1) {
                printf("Failed in reading a number");
            }
            token_val.ptr->type = Num;
            match(Sym);
        }
        match(')');
        match(';');
    }
    else if (token == Call) {
        match(token);
        match('(');
        char cmd[MAX_CMD_LENTH];
        if (token == UnnamedStr) {
            strcpy(cmd, token_val.str);
        }
        else if (token == NamedStr) {
            strcpy(cmd, token_val.ptr->pointer.funcp);
        }
        else {
            printf("Error: call: expects a string argument at line %.*s.\n", (int)(src - old_src), old_src);
            exit(1);
        }

        int tokens[] = {UnnamedStr, NamedStr};
        match_multiple(tokens, 2);
        match(',');

        if (token != Sym) {
            release_symbol(token_val.ptr);
        }
        token_val.ptr->pointer.funcp = (char*)malloc(sizeof(char)*CMD_OUT_BUF_LENGTH);
        token_val.ptr->type = NamedStr;

        int tokens2[] = {Sym, NamedStr, FuncSym, ArraySym};
        match_multiple(tokens2, 4);

        if (token_val.ptr->pointer.funcp == NULL) {
            printf("Failed in executing the shell command: %s.\n", cmd);
            exit(1);
        }

        printf("Execute the shell command: %s.\n", cmd);
        int res = shellcall(cmd, token_val.ptr->pointer.funcp, CMD_OUT_BUF_LENGTH);
        switch (res) {
            case shellcall_failed:
                puts("Execution failed.\n");
                break;
            case shellcall_truncate:
                puts("Execution success, but the result may be truncated.");
                break;
            case shellcall_success:
                puts("Execution success.");
                break;
            default:
                assert(0);
        }
        match(')');
        match(';');
    }
    else {
        printf("Error: unknown token at line %.*s.\n", (int)(src - old_src), old_src);
        exit(1);
    }
    return 0;
}

static double function() {
    currentlevel++;
    return_val = 0;

    symbol* s = token_val.ptr;
    match(FuncSym);
    match('(');
    while (token != ')') {
        symbol *cur = &symtab[symPointer];
        *cur = *token_val.ptr;
        strcpy(cur->name, token_val.ptr->name);
        cur->levelNum = currentlevel;
        if (cur->type == ArraySym) {
            int size = sizeof(struct symStruct) * cur->value + 1;
            symbol *new_list = (struct symStruct*)malloc(sizeof(struct symStruct) * cur->value + 1);
            memcpy(new_list, cur->pointer.list, size);
            cur->pointer.list = new_list;
        }
        else if (cur->type == NamedStr) {
            /* TODO: strlen */
            int size = strlen(cur->pointer.funcp) + 8;
            char *new_str = (char*)malloc(size);
            strcpy(new_str, cur->pointer.funcp);
            cur->pointer.funcp = new_str;
        }
        symPointer++;
        int tokens[3] = {Sym, ArraySym, NamedStr}; /* TODO: test for ArraySym and NamedStr */
        match_multiple(tokens, 3);
        if (token == ',')
            match(',');
    }
    match(')');
    char* startPos = src;
    char* startOldPos = old_src;
    int startToken = token;
    old_src = src = s->pointer.funcp;
    token = (int)s->value;
    statement();
    src = startPos;
    old_src = startOldPos;
    token = startToken;

    while (symPointer>=1 && symtab[symPointer-1].levelNum == currentlevel) {
        release_symbol(&symtab[symPointer-1]);
        symPointer--;
    }
    currentlevel--;
    return return_val;
}

static void release_symbol(symbol *psymbol) {
    int token_type = psymbol->type;
    if (token_type == NamedStr) {
        free(psymbol->pointer.funcp);
    }
    else if (token_type == ArraySym) {
        free(psymbol->pointer.funcp);
    }
}

static void index_error() {
    puts("Error: array index out of boundary.");
    exit(1);
}

static char *pool;

static void tryc_init() {
    break_flag = false;
    continue_flag = false;
    symPointer = 0;
    currentlevel = 0;
    return_val = 0.0;

    src = "array func else if return while print puts read continue break call";
    int i;
    for (i = Array; i <= Call; ++i) {
        next();
        symtab[symPointer-1].type = i;
    }
    pool = (char*)malloc(POOLSIZE);
    if (!pool) {
        printf("could not malloc(%d) for source area\n", POOLSIZE);
        exit(-1);
    }
    src = old_src = pool;
}

/*----------------------------------------------------------------*/

int tryc_exec(int argc, char** argv)
{
    tryc_init();
    int fd;
    ++argv; --argc;
    if (argc > 0) {
        if (**argv == '-' && (*argv)[1] == 'd') {
            compileState = debug;
            ++argv; --argc;
        }
        else {
            compileState = run;
        }
    }
    if (argc < 1) {
        printf("usage: tryc [-d] file ...\n");
        return -1;
    }

    if ((fd = open(*argv, 0)) < 0) {                /* read the source file */
        printf("could not open(%s)\n", *argv);
        return -1;
    }
    int i;
    if ((i = read(fd, src, POOLSIZE - 1)) <= 0) {
        printf("read() returned %d\n", i);
        return -1;
    }
    src[i] = 0; /* add EOF character */
    close(fd);
    next();
    while (token != 0) {
        statement();
    }

    free(pool);

    return 0;
}
