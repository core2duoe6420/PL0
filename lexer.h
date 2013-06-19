#ifndef __LEXICAL_H
#define __LEXICAL_H

#include <stdio.h>

#define ID_MAX_LEN 64
#define BUFFER_SIZE 4096
#define ST_INCREMENT 4096

#define TYPE_CONST 0
#define	TYPE_VAR 1
#define	TYPE_PROCEDURE 2
#define	TYPE_CALL 3
#define	TYPE_BEGIN 4
#define	TYPE_END 5
#define	TYPE_IF 6
#define TYPE_ELSE 7
#define TYPE_THEN 8
#define	TYPE_WHILE 9
#define	TYPE_DO 10
#define	TYPE_READ 11
#define	TYPE_WRITE 12
#define TYPE_ODD 13
#define	TYPE_IDENTIFIER 14
#define	TYPE_INTEGER 15
#define	TYPE_DOUBLE 16
#define	TYPE_SEQUAL 17
#define	TYPE_SNOTEQU 18
#define	TYPE_SASSIGN 19
#define	TYPE_SCOMMA 20
#define	TYPE_SLESS 21
#define	TYPE_SLESSEQU 22
#define	TYPE_SABOVE 23
#define	TYPE_SABOVEEQU 24
#define	TYPE_SSEMICOLON 25
#define TYPE_SEND 26
#define TYPE_SLBRACKETS 27
#define TYPE_SRBRACKETS 28
#define TYPE_APLUS 29
#define TYPE_AMINUS 30
#define TYPE_AMULTIPLY 31
#define TYPE_ADIVIDE 32
#define TYPE_CONSTASSIGN 33
#define	TYPE_UNDEFINED 34
#define TYPE_IDOVERLENGTH 35

#define TYPE_STAB 36

#define TYPE_NR (TYPE_STAB+1)

struct terminal {
    int word_type;
    char vt;
    char * print_name;

    int term_type;
    unsigned int hash;
    char * vt_name;
};

#define TERM_RW 0
#define TERM_SYM 1
#define TERM_NUM 2
#define TERM_IDENT 3
#define TERM_WRONG 4
#define TERM_STAB 5

#include "light_rbtree.h"

extern struct terminal terminals[];
extern int TERMINALS_NR;
extern light_rbtree terminals_rbtree;

extern void initial_terminals();

/* 所有identifier的字符串都存储在word_table->identifiers中
 * word结构体中只存放偏移量，首地址是word_table->identifiers.buffer
 */
struct word {
    struct terminal * type;
    union {
        int lex_offset;
        int ivalue;
        //double dvalue;
    };
    unsigned int ident_hash;
    int row;
    struct word_table * table;
};

struct string_table {
    int max;
    int offset;
    char * buffer;
};

struct word_table {
    int count;
    int max;
    struct word * words;
    struct string_table identifiers;
};

#define identname(w) (((w)->table->identifiers.buffer)+((w)->lex_offset))

struct buffer {
    FILE * file;
    int file_len;
    int offset;
    int buf_id;
    int next;
    char buf[2][BUFFER_SIZE];
};

struct buffer * buffer_create(FILE * file);
char buffer_get_next(struct buffer * buf);
void buffer_put_back(struct buffer * buf);
struct word_table * word_table_create();
void buffer_drop(struct buffer ** buf);
void word_table_drop(struct word_table ** wt);
int word_table_add(struct word_table * wt, struct word * w, char * id_name);
int string_table_add(struct string_table * st, char * str);
struct word_table * analyze_lexical(struct buffer * buf);
void word_table_print(struct word_table * wt);
extern inline char * word_to_string(struct word * w);
extern inline struct terminal * get_terminal(char * key);
unsigned int get_string_hash(char * str);
#endif