#ifndef __MID_CODE_H
#define __MID_CODE_H

#include "sym_table.h"

struct mid_statement {
    int id;
    int type;
    struct symbol * dest;
    struct symbol * src;
};

struct statements_set {
    int count;
    int max;
    struct mid_statement * statements;
};

#define MIDOP_ASSIGN 1
#define MIDOP_ALLOC 2
#define MIDOP_ADD 3
#define MIDOP_MINUS 4
#define MIDOP_MULTIPLY 5
#define MIDOP_DIVIDE 6
#define MIDOP_IF 7
#define MIDOP_GOTO 8
#define MIDOP_CREAT_PROC 9
#define MIDOP_END_PROC 10
#define MIDOP_CALL 11
#define MIDOP_NEG 12
#define MIDOP_GETTEMP 13
#define MIDOP_DROPTEMP 14
#define MIDOP_MOD 15
#define MIDOP_COMPARE 16
#define MIDOP_SET_COMPARE_METHOD 17
#define MIDOP_CREATE_LABEL 18
#define MIDOP_READ 19
#define MIDOP_WRITE 20

struct mid_statement * mid_statement_insert(struct statements_set * ss,
        int type, struct symbol * dest,
        struct symbol * src);
struct statements_set * statements_set_create();
void statements_set_print(struct statements_set * ss);
void statements_set_print_all_scope(struct scope * scp);
void statements_set_drop(struct statements_set * ss);

#endif