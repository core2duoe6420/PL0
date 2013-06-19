#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"

#define is_digit(d) (d>='0' && d<='9')
#define is_char(c) ((c>='a' && c<='z') || (c>='A' && c<='Z'))
#define is_blank(c) (c==' ' || c==13 || c=='\t')
#define is_lf(c) (c==10)
#define is_dot(c) (c=='.')

#define is_sym(c) ((c==',' || c==';' || c==':' || c=='=' || c=='<') \
	|| (c=='>' || c=='#' || c=='+' || c=='-' || c=='*' || c=='/')	\
	|| (c=='(' || c== ')'))


unsigned int get_string_hash(char * str)
{
    unsigned int hash = 0;
    char ch;
    while(ch = *str++)
        hash = hash * 131 + ch;
    return hash;
}

inline char * word_to_string(struct word * w)
{
    if(w->type->word_type == TYPE_IDENTIFIER)
        return identname(w);
    return w->type->print_name;
}

inline struct terminal * get_terminal(char * key) {
    int hash = get_string_hash(key);
    struct terminal * ter;
    ter = (struct terminal *)light_rbtree_get_ele(terminals_rbtree, &hash);
    return ter;
}

static struct terminal * check_reserved_word(struct word * w, char * str) {
    static unsigned int ident_hash = 0;
    unsigned int hash;
    struct terminal * ter;
    if(!ident_hash)
        ident_hash = get_string_hash("ident");

    hash = get_string_hash(str);
    w->ident_hash = hash;

    ter = (struct terminal *)light_rbtree_get_ele(terminals_rbtree, &hash);

    if(!ter || ter->term_type != TERM_RW)
        return (struct terminal *)light_rbtree_get_ele(terminals_rbtree, &ident_hash);

    if(strcmp(str, ter->vt_name) == 0)
        return ter;
    else
        return (struct terminal *)light_rbtree_get_ele(terminals_rbtree, &ident_hash);
}

static struct terminal * check_symbol(char * str) {
    unsigned int hash;
    struct terminal * ter;

    hash = get_string_hash(str);

    ter = (struct terminal *)light_rbtree_get_ele(terminals_rbtree, &hash);

    if(!ter || ter->term_type != TERM_SYM)
        return NULL;

    if(strcmp(str, ter->vt_name) == 0)
        return ter;
    else
        return NULL;
}

static void get_error(struct buffer * buf, struct word * w, char * lex, char start, int idx, int row)
{
    char ch;
    lex[idx++] = start;

    while((ch = buffer_get_next(buf)) != EOF) {
        if(is_blank(ch)) {
            break;
        } else if(is_lf(ch) || is_sym(ch) || is_dot(ch)) {
            buffer_put_back(buf);
            break;
        }
        lex[idx++] = ch;
    }

    lex[idx] = 0;
    if(idx >= ID_MAX_LEN - 1)
        w->type = get_terminal("idoverlength");
    else
        w->type = get_terminal("undefined");

    w->row = row;
}

static void get_identifier(struct buffer * buf, struct word * w, char * lex, int row)
{
    int idx = 1;
    char ch;

    while((ch = buffer_get_next(buf)) != EOF) {
        if(is_blank(ch))
            break;

        if(is_lf(ch) || is_sym(ch) || is_dot(ch)) {
            buffer_put_back(buf);
            break;
        }

        //超出限制长度？
        if(idx >= ID_MAX_LEN) {
            get_error(buf, w, lex, ch, idx, row);
            return;
        }

        if(is_char(ch) || is_digit(ch) || ch == '_') {
            lex[idx++] = ch;
        } else {
            //执行到这里说明出现了非法字符
            get_error(buf, w, lex, ch, idx, row);
            return;
        }
    }

    lex[idx] = 0;

    w->type = check_reserved_word(w, lex);
    w->row = row;
}

static void get_number(struct buffer * buf, struct word * w, char * lex, int row)
{
    char ch;
    int idx = 1;
    int type = TYPE_UNDEFINED;

    while((ch = buffer_get_next(buf)) != EOF) {
        if(is_blank(ch))
            break;

        if(is_lf(ch) || is_sym(ch)) {
            buffer_put_back(buf);
            break;
        }

        if(idx >= ID_MAX_LEN) {
            get_error(buf, w, lex, ch, idx, row);
            return;
        }

        if(is_digit(ch)) {
            lex[idx++] = ch;
        } else {
            //执行到这里说明出现了非法字符
            get_error(buf, w, lex, ch, idx, row);
            return;
        }
    }

    lex[idx] = 0;

    type = TYPE_INTEGER;
    w->ivalue = atoi(lex);
    w->type = get_terminal("integer");

    w->row = row;
}

static void get_sym(struct buffer * buf, struct word * w, char * lex, int row)
{
    char ch;
    int need_put_back = 1;
    int idx = 1;

    ch = buffer_get_next(buf);

    switch(lex[0]) {
    case ':':
        if(ch != '=') {
            get_error(buf, w, lex, ch, idx, row);
            return;
        } else {
            lex[idx++] = ch;
            need_put_back = 0;
        }
        break;
    case '<':
        if(ch == '=') {
            lex[idx++] = ch;
            need_put_back = 0;
        }
        break;
    case '>':
        if(ch == '=') {
            lex[idx++] = ch;
            need_put_back = 0;
        }
        break;
    case '=':
        if(ch == '=') {
            lex[idx++] = ch;
            need_put_back = 0;
        }
        break;
    default:
        break;
    }

    if(ch != EOF && need_put_back)
        buffer_put_back(buf);

    lex[idx] = 0;
    w->row = row;
    w->type = check_symbol(lex);
}

struct word_table * analyze_lexical(struct buffer * buf) {
    char ch;
    char lex[4096] = {0,};
    int row = 1;
    struct word w;
    struct word_table * wt;

    initial_terminals();
    wt = word_table_create();

    while((ch = buffer_get_next(buf)) != EOF) {
        lex[0] = ch;
        if(is_lf(ch)) {
            row++;
            continue;
        } else if(is_blank(ch)) {
            continue;
            //什么都不做，跳过而已
        } else if(is_char(ch)) {
            get_identifier(buf, &w, lex, row);
        } else if(is_digit(ch)) {
            get_number(buf, &w, lex, row);
        } else if(is_sym(ch)) {
            get_sym(buf, &w, lex, row);
        } else if(is_dot(ch)) {
            w.type = get_terminal(".");
        } else {
            //如果执行到这里表示出现了无法识别的符号
            get_error(buf, &w, lex, ch, 0, row);
        }
        word_table_add(wt, &w, lex);
    }
    //加入一个桩符号表示结束
    w.type = get_terminal("stab");
    word_table_add(wt, &w, NULL);
    return wt;
}