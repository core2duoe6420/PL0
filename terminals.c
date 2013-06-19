#include "lexer.h"
#include "parser.h"
#include "light_rbtree.h"

unsigned int get_string_hash(char * str);

light_rbtree terminals_rbtree;

static int terminals_cmp_ele(void * ter_a, void * ter_b)
{
    return ((struct terminal *)ter_a)->hash - ((struct terminal *)ter_b)->hash;
}

static int terminals_cmp_key(void * ter, void * key)
{
    return ((struct terminal *)ter)->hash - *(int *)key;
}

void initial_terminals()
{
    terminals_rbtree = light_rbtree_create(sizeof(struct terminal),
                                           terminals_cmp_ele,
                                           terminals_cmp_key,
                                           NULL,
                                           NULL);
    for(int i = 0; i < TERMINALS_NR; i++) {
        terminals[i].hash = get_string_hash(terminals[i].vt_name);
        light_rbtree_insert(terminals_rbtree, &terminals[i]);
    }
}

struct terminal terminals[] = {
    {TYPE_CONST, VT_CONST, "const", TERM_RW, 0, "const"},
    {TYPE_VAR, VT_VAR, "var", TERM_RW, 0, "var"},
    {TYPE_PROCEDURE, VT_PROCEDURE, "procedure", TERM_RW, 0, "procedure"},
    {TYPE_CALL, VT_CALL, "call", TERM_RW, 0, "call"},
    {TYPE_BEGIN, VT_BEGIN, "begin", TERM_RW, 0, "begin"},
    {TYPE_END, VT_END, "end", TERM_RW, 0, "end"},
    {TYPE_IF, VT_IF, "if", TERM_RW, 0, "if"},
    {TYPE_THEN, VT_THEN, "then", TERM_RW, 0, "then"},
    {TYPE_ELSE, VT_ELSE, "else", TERM_RW, 0, "else"},
    {TYPE_WHILE, VT_WHILE, "while", TERM_RW, 0, "while"},
    {TYPE_DO, VT_DO, "do", TERM_RW, 0, "do"},
    {TYPE_READ, VT_READ, "read", TERM_RW, 0, "read"},
    {TYPE_WRITE, VT_WRITE, "write", TERM_RW, 0, "write"},
    {TYPE_ODD, VT_ODD, "odd", TERM_RW, 0, "odd"},
    {TYPE_IDENTIFIER, VT_IDENTIFIER, "identifier", TERM_IDENT, 0, "ident"},
    {TYPE_INTEGER, VT_NUM, "integer", TERM_NUM, 0, "integer"},
    //{TYPE_DOUBLE, VT_NUM, "double", TERM_NUM, 0, "double"},
    {TYPE_SEQUAL, VT_COMPARE, "==", TERM_SYM, 0, "=="},
    {TYPE_SNOTEQU, VT_COMPARE, "#", TERM_SYM, 0, "#"},
    {TYPE_SASSIGN, VT_ASSIGN, ":=", TERM_SYM, 0, ":="},
    {TYPE_SCOMMA, VT_COMMA, ",", TERM_SYM, 0, ","},
    {TYPE_SLESS, VT_COMPARE, "<", TERM_SYM, 0, "<"},
    {TYPE_SLESSEQU, VT_COMPARE, "<=", TERM_SYM, 0, "<="},
    {TYPE_SABOVE, VT_COMPARE, ">", TERM_SYM, 0, ">"},
    {TYPE_SABOVEEQU, VT_COMPARE, ">=", TERM_SYM, 0, ">="},
    {TYPE_SSEMICOLON, VT_SEMICOLON, ";", TERM_SYM, 0, ";"},
    {TYPE_SEND, VT_PROC_END, ".", TERM_SYM, 0, "."},
    {TYPE_SLBRACKETS, VT_LBRACKETS, "(", TERM_SYM, 0, "("},
    {TYPE_SRBRACKETS, VT_RBRACKETS, ")", TERM_SYM, 0, ")"},
    {TYPE_APLUS, VT_PLUS_MINUS, "+", TERM_SYM, 0, "+"},
    {TYPE_AMINUS, VT_PLUS_MINUS, "-", TERM_SYM, 0, "-"},
    {TYPE_AMULTIPLY, VT_MUL_DIV, "*", TERM_SYM, 0, "*"},
    {TYPE_ADIVIDE, VT_MUL_DIV, "/", TERM_SYM, 0, "/"},
    {TYPE_CONSTASSIGN, VT_CONSTASSIGN, "=", TERM_SYM, 0, "="},
    {TYPE_UNDEFINED, 0, "undefined", TERM_WRONG, 0, "undefined"},
    {TYPE_IDOVERLENGTH, 0, "idoverlength", TERM_WRONG, 0, "idoverlength"},
    {TYPE_STAB, VT_STAB, "stab", TERM_STAB, 0, "stab"},
};

int TERMINALS_NR = ((sizeof(terminals)) / sizeof(struct terminal));