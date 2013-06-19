#include "midcode.h"
#include <stdlib.h>

#define STATEMENT_INC 1024

struct statements_set * statements_set_create() {
    struct statements_set * ss;
    ss = (struct statements_set *)malloc(sizeof(struct statements_set));
    if(!ss)
        return NULL;

    ss->count = 0;
    ss->max = STATEMENT_INC;
    ss->statements = (struct mid_statement *)malloc(sizeof(struct mid_statement)
                     * STATEMENT_INC);
    if(!ss->statements) {
        free(ss);
        return NULL;
    }
    return ss;
}

void statements_set_drop(struct statements_set * ss)
{
    free(ss->statements);
    free(ss);
}

struct mid_statement * mid_statement_get_instance(struct statements_set * ss) {
    if(ss->count == ss->max) {
        ss->statements = (struct mid_statement *)realloc(ss->statements,
                         sizeof(struct mid_statement) * (ss->max + STATEMENT_INC));
        if(!ss->statements)
            return NULL;
        ss->max += STATEMENT_INC;
    }

    struct mid_statement * ms;
    ms = ss->statements + ss->count;
    ms->id = ss->count;
    ss->count++;
    return ms;
}

struct mid_statement * mid_statement_insert(struct statements_set * ss,
        int type, struct symbol * dest,
        struct symbol * src) {
    struct mid_statement * ms;
    ms = mid_statement_get_instance(ss);
    if(!ms)
        return NULL;

    ms->type = type;
    ms->dest = dest;
    ms->src = src;
    return ms;
}

void statements_set_print(struct statements_set * ss)
{
    for(int i = 0; i < ss->count; i++) {
        struct mid_statement * ms;
        ms = &ss->statements[i];
        char * name, * dest, * src;
        dest = ms->dest ? ms->dest->name : "";
        src = ms->src ? ms->src->name : "";
        switch(ms->type) {
        case MIDOP_ALLOC:
            name = "ALLOC";
            break;
        case MIDOP_CREAT_PROC:
            printf("\n");
            name = "CREATE PROC";
            break;
        case MIDOP_END_PROC:
            name = "END PROC";
            break;
        case MIDOP_CALL:
            name = "CALL";
            break;
        case MIDOP_ASSIGN:
            name = ":=";
            break;
        case MIDOP_NEG:
            name = "NEG";
            break;
        case MIDOP_ADD:
            name = "+";
            break;
        case MIDOP_MINUS:
            name = "-";
            break;
        case MIDOP_MULTIPLY:
            name = "*";
            break;
        case MIDOP_DIVIDE:
            name = "/";
            break;
        case MIDOP_GETTEMP:
            name = "GET TEMP";
            break;
        case MIDOP_DROPTEMP:
            name = "DROP TEMP";
            break;
        case MIDOP_MOD:
            name = "%";
            break;
        case MIDOP_COMPARE:
            name = "COMPARE";
            break;
        case MIDOP_SET_COMPARE_METHOD:
            name = "COMPARE METHOD";
            break;
        case MIDOP_CREATE_LABEL:
            name = "LABEL";
            break;
        case MIDOP_IF:
            name = "IF FALSE";
            break;
        case MIDOP_GOTO:
            name = "GOTO";
            break;
        case MIDOP_READ:
            name = "READ";
            break;
        case MIDOP_WRITE:
            name = "WRITE";
            break;
        }
        printf("%-15s%-10s%-10s\n", name, dest, src);
    }
}

void statements_set_print_all_scope(struct scope * scp)
{
    if(!scp)
        return;
    statements_set_print(scp->ss);
    statements_set_print_all_scope(scp->sibling);
    statements_set_print_all_scope(scp->child);
}