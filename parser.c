/* PL0语法分析 递归子程序法
* 产生式：
* 程序 => 分程序 .
* 分程序 => 常量声明 变量声明 过程声明 语句
* 常量声明 => ε
* 常量声明 => const ident = num 常量part ;
* 常量part => ε
* 常量part => , ident = num 常量part
* 变量声明 => ε
* 变量声明 => var ident 变量part ;
* 变量part => ε
* 变量part => , ident 变量part
* 过程声明 => ε
* 过程声明 => procedure ident ; 分程序 ; 过程声明
* 语句 => ident := 表达式 ;
* 语句 => call ident
* 语句 => { 语句 语句part1 }
* 语句part1 => ε
* 语句part1 => ; 语句 语句part1
* 语句 => while 条件 do 语句
* 语句 => read ( ident 变量part )
* 语句 => write ( 表达式 语句part2 )
* 语句part2 => ε
* 语句part2 => , 表达式
* 语句 => if 条件 then 语句 语句part3
* 语句part3 => ε
* 语句part3 => else 语句
* 条件 => odd 表达式
* 条件 => 表达式 φ 表达式
* 表达式 => μ 表达式part1
* 表达式 => 表达式part1
* 表达式part1 => 项 表达式part2
* 表达式part2 => ε
* 表达式part2 => μ 项 表达式part2
* 项 => 因子 项part
* 项part => ε
* 项part => ω 因子 项part
* 因子 => ident
* 因子 => num
* 因子 => ( 表达式 )

* φ：比较运算符 == # < <= > >=
* μ：加减号 + -
* ω：乘除号 * /

* 非终结符：
* A程序
* B分程序
* C常量声明
* D变量声明
* E过程声明
* F语句
* G常量part
* H变量part
* I表达式
* J语句part1
* K语句part2
* L语句part3
* M条件
* N表达式part1
* O表达式part2
* P项
* Q因子
* R项part
*/

/* 修改的地方：
*  加入else语句
*/

/* 好吧，我承认我滥用预处理了，
* 不过至少让代码看上去漂亮了点:-)
*/


#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "sym_table.h"
#include "midcode.h"

static int parse_factor(struct word_table * wt, int * idx, struct symbol * sym, int type);
static int parse_item(struct word_table * wt, int * idx, struct symbol * sym);
static int parse_expression(struct word_table * wt, int * idx, struct symbol * sym);
static int parse_condition(struct word_table * wt, int * idx, struct symbol * sym);
static int parse_statement(struct word_table * wt, int * idx, struct symbol * sym);
static int parse_procedure(struct word_table * wt, int * idx, struct symbol * sym);
static int parse_var(struct word_table * wt, int * idx, struct symbol * sym);
static int parse_const(struct word_table * wt, int * idx, struct symbol * sym);
static int parse_div_program(struct word_table * wt, int * idx, struct symbol * sym);
static int parse_program(struct word_table * wt, int * idx, struct symbol * sym);

struct scope * cur_scope;

#define get_type_inc(wt,idx_ptr) ((wt)->words[(*(idx_ptr))++].type->vt)
#define get_type(wt,idx_ptr) ((wt)->words[(*(idx_ptr))].type->vt)

#define check_type_inc_idx(wt,idx,type,err_info,errcode) \
	if(get_type_inc((wt),(idx)) != (type)) { \
	print_err_missing((err_info),(&(wt)->words[(*(idx))])); \
	return (errcode); \
	}

#define check_subroutine_ret(ret,parse_func,wt,idx,sym) \
	if((ret)=parse_func((wt),(idx),(sym))) \
	return (ret)

#define check_ident_type(ret,type,wt,idx,sym) \
	if((sym=check_ident_##type((wt),(idx))) == NULL) { \
	ret = 1; \
	return (ret);	\
	}\
 

#define check_ident_add_sym_table(ret,type,sym,wt,idx) \
	if((ret) = _check_ident_add_symtable((wt), (idx), (type),(sym) )) \
	return (ret);

static inline void print_err_expect(char * expect, struct word * w)
{
    printf("Error row %d : expect %s but meet %s\n",
           w->row, expect, word_to_string(w));
}

static inline void print_err_missing(char * missing, struct word * w)
{
    printf("Error row %d : missing expected %s\n",
           w->row, missing);
}

int analyze_parse_rec_sub(struct word_table * wt)
{
    int err;
    int idx = 0;

    initial_scope();
    cur_scope = global_scope;

    err = parse_program(wt, &idx, NULL);
    return err;
}

static int _check_ident_add_symtable(struct word_table * wt, int * idx,
                                     int type, struct symbol ** sym)
{
    struct symbol * s;
    struct word * w;
    int err = 0, ridx;

    ridx = *idx;
    w = &wt->words[ridx];

    check_type_inc_idx(wt, idx, VT_IDENTIFIER, "identifier", 1);

    s = scope_add_symbol(cur_scope, identname(w), type, &err);
    if(err)
        printf("Error row %d : %s redefined\n", wt->words[ridx].row, identname(w));
    if(sym)
        *sym = s;
    return err;
}

static struct symbol * _check_ident(struct word_table * wt, int * idx, int type) {
    struct symbol * sym;
    int err = 0, ridx;
    char * errinfo;
    struct word * w;

    ridx = *idx;
    w = &wt->words[ridx];

    if(get_type_inc(wt, idx) != VT_IDENTIFIER) {
        print_err_missing("identifier", &wt->words[*idx]);
        return NULL;
    }

    if(!(sym = scope_get_symbol_by_word(cur_scope, w))) {
        printf("Error row %d : undefined symbol \"%s\"\n", w->row, identname(w));
        return NULL;
    }

    switch(type) {
    case SYM_TYPE_PROC:
        errinfo = "procedure name";
        break;
    case SYM_TYPE_CONST:
        errinfo = "const value";
        break;
    case SYM_TYPE_VAR:
        errinfo = "variable";
        break;
    case SYM_TYPE_VAR_CONST:
        errinfo = "variable or const value";
        break;
    }

    if(type != SYM_TYPE_VAR_CONST && sym->type != type) {
        print_err_expect(errinfo, w);
        return NULL;
    } else if(type == SYM_TYPE_VAR_CONST) {
        if(sym->type != SYM_TYPE_CONST && sym->type != SYM_TYPE_VAR) {
            print_err_expect(errinfo, w);
            return NULL;
        }
    }

    return sym;
}

static struct symbol * check_ident_var(struct word_table * wt, int * idx) {
    return _check_ident(wt, idx, SYM_TYPE_VAR);
}

static struct symbol * check_ident_proc(struct word_table * wt, int * idx) {
    return _check_ident(wt, idx, SYM_TYPE_PROC);
}

static struct symbol * check_ident_const(struct word_table * wt, int * idx) {
    return _check_ident(wt, idx, SYM_TYPE_CONST);
}

static struct symbol * check_ident_const_var(struct word_table * wt, int * idx) {
    return _check_ident(wt, idx, SYM_TYPE_VAR_CONST);
}

static struct symbol * get_cur_scope_symbol() {
    struct symbol * scope_sym = NULL;
    if(cur_scope != global_scope)
        scope_sym = scope_get_symbol_by_name(cur_scope->father, cur_scope->name);
    return scope_sym;
}

static int parse_program(struct word_table * wt, int * idx, struct symbol * sym)
{
    int err = 0;
    check_subroutine_ret(err, parse_div_program, wt, idx, NULL);
    check_type_inc_idx(wt, idx, VT_PROC_END, "'.'", 1);
    return err;
}

static int parse_div_program(struct word_table * wt, int * idx, struct symbol * sym)
{
    int err = 0;
    if(get_type(wt, idx) == VT_CONST)
        check_subroutine_ret(err, parse_const, wt, idx, NULL);

    if(get_type(wt, idx) == VT_VAR)
        check_subroutine_ret(err, parse_var, wt, idx, NULL);

    if(get_type(wt, idx) == VT_PROCEDURE)
        check_subroutine_ret(err, parse_procedure, wt, idx, NULL);
    return parse_statement(wt, idx, NULL);
}

static int parse_const(struct word_table * wt, int * idx, struct symbol * sym)
{
    int err = 0;
    struct symbol * const_sym;
    check_type_inc_idx(wt, idx, VT_CONST, "const", 1);
    check_ident_add_sym_table(err, SYM_TYPE_CONST, &const_sym, wt, idx);
    check_type_inc_idx(wt, idx, VT_CONSTASSIGN, "'='", 1);
    check_type_inc_idx(wt, idx, VT_NUM, "number", 1);
    const_sym->const_value = wt->words[*idx - 1].ivalue;

    while(get_type_inc(wt, idx) == VT_COMMA) {
        check_ident_add_sym_table(err, SYM_TYPE_CONST, &const_sym, wt, idx);
        check_type_inc_idx(wt, idx, VT_CONSTASSIGN, "'='", 1);
        check_type_inc_idx(wt, idx, VT_NUM, "number", 1);
        const_sym->const_value = wt->words[*idx - 1].ivalue;
    }
    (*idx)--;
    check_type_inc_idx(wt, idx, VT_SEMICOLON, "';'", 1);
    return err;
}

static int parse_var(struct word_table * wt, int * idx, struct symbol * sym)
{
    int err = 0;
    struct symbol * var_sym, * scope_sym;
    check_type_inc_idx(wt, idx, VT_VAR, "var", 1);
    check_ident_add_sym_table(err, SYM_TYPE_VAR, &var_sym, wt, idx);

    scope_sym = get_cur_scope_symbol();

    mid_statement_insert(cur_scope->ss, MIDOP_ALLOC, var_sym, scope_sym);

    while(get_type_inc(wt, idx) == VT_COMMA) {
        check_ident_add_sym_table(err, SYM_TYPE_VAR, &var_sym, wt, idx);
        mid_statement_insert(cur_scope->ss, MIDOP_ALLOC, var_sym, scope_sym);
    }

    (*idx)--;
    check_type_inc_idx(wt, idx, VT_SEMICOLON, "';'", 1);
    return err;
}

static int parse_procedure(struct word_table * wt, int * idx, struct symbol * sym)
{
    int err = 0;
    struct symbol * proc, * father;
    struct scope * new_scp;

    check_type_inc_idx(wt, idx, VT_PROCEDURE, "procedure", 1);
    check_ident_add_sym_table(err, SYM_TYPE_PROC, &proc, wt, idx);
    if(proc) {
        father = get_cur_scope_symbol();
        new_scp = scope_new(cur_scope, proc);
        cur_scope = new_scp;
        mid_statement_insert(cur_scope->ss, MIDOP_CREAT_PROC, proc, father);
    }
    check_type_inc_idx(wt, idx, VT_SEMICOLON, "';'", 1);
    check_subroutine_ret(err, parse_div_program, wt, idx, NULL);
    check_type_inc_idx(wt, idx, VT_SEMICOLON, "';'", 1);
    if(proc) {
        mid_statement_insert(cur_scope->ss, MIDOP_END_PROC, proc, father);
        cur_scope = cur_scope->father;
    }

    if(get_type(wt, idx) == VT_PROCEDURE)
        check_subroutine_ret(err, parse_procedure, wt, idx, NULL);
    return err;
}

static int parse_statement_identifier(struct word_table * wt, int * idx)
{
    int err = 0;
    struct symbol * ident_sym, *exp_value;
    (*idx)--;
    check_ident_type(err, var, wt, idx, ident_sym);
    check_type_inc_idx(wt, idx, VT_ASSIGN, "':='", 1);
    exp_value = symbol_create_temp();
    mid_statement_insert(cur_scope->ss, MIDOP_GETTEMP, exp_value, NULL);
    check_subroutine_ret(err, parse_expression, wt, idx, exp_value);
    mid_statement_insert(cur_scope->ss, MIDOP_ASSIGN, ident_sym, exp_value);
    mid_statement_insert(cur_scope->ss, MIDOP_DROPTEMP, exp_value, NULL);
    ident_sym->initialized = 1;
    return 0;
}

static int parse_statement_call(struct word_table * wt, int * idx)
{
    int err = 0;
    struct symbol * ident_sym, * scope_sym;
    check_ident_type(err, proc, wt, idx, ident_sym);
    scope_sym = get_cur_scope_symbol();
    mid_statement_insert(cur_scope->ss, MIDOP_CALL, ident_sym, NULL);
    return 0;
}

static int parse_statement_block(struct word_table * wt, int * idx)
{
    int err = 0;
    check_subroutine_ret(err, parse_statement, wt, idx, NULL);

    while(get_type_inc(wt, idx) == VT_SEMICOLON)
        check_subroutine_ret(err, parse_statement, wt, idx, NULL);

    (*idx)--;
    check_type_inc_idx(wt, idx, VT_END, "end", 1);
    return 0;
}

static int parse_statement_while(struct word_table * wt, int * idx)
{
    int err = 0;
    struct symbol * label, * cond_value;
    struct mid_statement * ms;

    label = symbol_create_label();
    cond_value = symbol_create_temp();
    mid_statement_insert(cur_scope->ss, MIDOP_GETTEMP, cond_value, NULL);

    check_subroutine_ret(err, parse_condition, wt, idx, cond_value);
    ms = mid_statement_insert(cur_scope->ss, MIDOP_IF, NULL, NULL);
    mid_statement_insert(cur_scope->ss, MIDOP_DROPTEMP, cond_value, NULL);
    check_type_inc_idx(wt, idx, VT_DO, "do", 1);
    check_subroutine_ret(err, parse_statement, wt, idx, NULL);
    mid_statement_insert(cur_scope->ss, MIDOP_GOTO, label, NULL);
    label = symbol_create_label();
    ms->dest = label;
    return 0;
}

static int parse_statement_read(struct word_table * wt, int * idx)
{
    int err = 0;
    struct symbol * ident_sym;
    check_type_inc_idx(wt, idx, VT_LBRACKETS, "'('", 1);
    check_ident_type(err, var, wt, idx, ident_sym);
    mid_statement_insert(cur_scope->ss, MIDOP_READ, ident_sym, NULL);
    ident_sym->initialized = 1;
    while(get_type_inc(wt, idx) == VT_COMMA) {
        check_ident_type(err, var, wt, idx, ident_sym);
        mid_statement_insert(cur_scope->ss, MIDOP_READ, ident_sym, NULL);
        ident_sym->initialized = 1;
    }
    (*idx)--;
    check_type_inc_idx(wt, idx, VT_RBRACKETS, "')'", 1);
    return 0;
}

static int parse_statement_write(struct word_table * wt, int * idx)
{
    int err = 0;
    struct symbol * exp_value;
    check_type_inc_idx(wt, idx, VT_LBRACKETS, "'('", 1);
    exp_value = symbol_create_temp();
    mid_statement_insert(cur_scope->ss, MIDOP_GETTEMP, exp_value, NULL);

    check_subroutine_ret(err, parse_expression, wt, idx, exp_value);
    mid_statement_insert(cur_scope->ss, MIDOP_WRITE, exp_value, NULL);
    mid_statement_insert(cur_scope->ss, MIDOP_DROPTEMP, exp_value, NULL);
    check_type_inc_idx(wt, idx, VT_RBRACKETS, "')'", 1);
    return 0;
}

static int parse_statement_if(struct word_table * wt, int * idx)
{
    int err = 0;
    struct symbol * cond_value, * label;
    struct mid_statement * ms1, * ms2;
    cond_value = symbol_create_temp();
    mid_statement_insert(cur_scope->ss, MIDOP_GETTEMP, cond_value, NULL);

    check_subroutine_ret(err, parse_condition, wt, idx, cond_value);
    ms1 = mid_statement_insert(cur_scope->ss, MIDOP_IF, NULL, NULL);
    mid_statement_insert(cur_scope->ss, MIDOP_DROPTEMP, cond_value, NULL);
    check_type_inc_idx(wt, idx, VT_THEN, "then", 1);
    check_subroutine_ret(err, parse_statement, wt, idx, NULL);

    if(get_type(wt, idx) == VT_ELSE)
        ms2 = mid_statement_insert(cur_scope->ss, MIDOP_GOTO, NULL, NULL);

    label = symbol_create_label();
    ms1->dest = label;

    if(get_type(wt, idx) == VT_ELSE) {
        (*idx)++;
        check_subroutine_ret(err, parse_statement, wt, idx, NULL);
        label = symbol_create_label();
        ms2->dest = label;
    }
    return 0;
}

static int parse_statement(struct word_table * wt, int * idx, struct symbol * sym)
{
    switch(get_type_inc(wt, idx)) {
    case VT_IDENTIFIER:
        return parse_statement_identifier(wt, idx);
    case VT_CALL:
        return parse_statement_call(wt, idx);
    case VT_BEGIN:
        return parse_statement_block(wt, idx);
    case VT_WHILE:
        return parse_statement_while(wt, idx);
    case VT_READ:
        return parse_statement_read(wt, idx);
    case VT_WRITE:
        return parse_statement_write(wt, idx);
    case VT_IF:
        return parse_statement_if(wt, idx);
    default:
        print_err_expect("identifier, call, while, read, write, if, begin",
                         &wt->words[*idx - 1]);
        return 1;
    }
}

static int parse_condition(struct word_table * wt, int * idx, struct symbol * sym)
{
    int err;
    struct symbol * value;
    if(get_type(wt, idx) == VT_ODD) {
        (*idx)++;
        check_subroutine_ret(err, parse_expression, wt, idx, sym);

        value = symbol_create_number(2);
        mid_statement_insert(cur_scope->ss, MIDOP_MOD, sym, value);
        value = symbol_create_number(1);
        mid_statement_insert(cur_scope->ss, MIDOP_COMPARE, sym, value);
        value = symbol_create_string("==");
        mid_statement_insert(cur_scope->ss, MIDOP_SET_COMPARE_METHOD, value, NULL);
    } else {
        check_subroutine_ret(err, parse_expression, wt, idx, sym);
        struct word * cmp = &wt->words[*idx];
        check_type_inc_idx(wt, idx, VT_COMPARE, "'==' '>' '>= '<' '<= '#'", 1);
        value = symbol_create_temp();
        mid_statement_insert(cur_scope->ss, MIDOP_GETTEMP, value, NULL);

        check_subroutine_ret(err, parse_expression, wt, idx, value);
        mid_statement_insert(cur_scope->ss, MIDOP_COMPARE, sym, value);
        mid_statement_insert(cur_scope->ss, MIDOP_DROPTEMP, value, NULL);
        value = symbol_create_string(cmp->type->vt_name);
        mid_statement_insert(cur_scope->ss, MIDOP_SET_COMPARE_METHOD, value, NULL);

    }
    return 0;
}

static int parse_expression(struct word_table * wt, int * idx, struct symbol * sym)
{
    int err, neg = 0;
    if(get_type(wt, idx) == VT_PLUS_MINUS) {
        if(wt->words[*idx].type->word_type == TYPE_AMINUS)
            neg = 1;
        (*idx)++;
    }

    int optype;
    struct symbol * item_value = NULL;
    struct word * w;

    check_subroutine_ret(err, parse_item, wt, idx, sym);

    //如果表达式要取反，必须在求值后进行
    if(neg)
        mid_statement_insert(cur_scope->ss, MIDOP_NEG, sym, NULL);

    while(get_type_inc(wt, idx) == VT_PLUS_MINUS) {
        if(!item_value) {
            item_value = symbol_create_temp();
            mid_statement_insert(cur_scope->ss, MIDOP_GETTEMP, item_value, NULL);
        }
        w = &wt->words[*idx - 1];
        if(w->type->word_type == TYPE_APLUS)
            optype = MIDOP_ADD;
        else if(w->type->word_type == TYPE_AMINUS)
            optype = MIDOP_MINUS;
        check_subroutine_ret(err, parse_item, wt, idx, item_value);
        mid_statement_insert(cur_scope->ss, optype, sym, item_value);
    }
    (*idx)--;
    if(item_value)
        mid_statement_insert(cur_scope->ss, MIDOP_DROPTEMP, item_value, NULL);

    return 0;
}

static int parse_item(struct word_table * wt, int * idx, struct symbol * sym)
{
    int err;
    int optype;
    struct word * w;

    if(parse_factor(wt, idx, sym, MIDOP_ASSIGN))
        return 1;

    while(get_type_inc(wt, idx) == VT_MUL_DIV) {
        w = &wt->words[*idx - 1];
        if(w->type->word_type == TYPE_AMULTIPLY)
            optype = MIDOP_MULTIPLY;
        else if(w->type->word_type == TYPE_ADIVIDE)
            optype = MIDOP_DIVIDE;
        if(parse_factor(wt, idx, sym, optype))
            return 1;
    }
    (*idx)--;
    return 0;
}

static int parse_factor(struct word_table * wt, int * idx, struct symbol * sym, int type)
{
    int err = 0;
    struct symbol * ident_sym, * value;
    struct word * w;

    w = &wt->words[*idx];
    switch(get_type(wt, idx)) {
    case VT_IDENTIFIER:
        err = (ident_sym = check_ident_const_var(wt, idx)) ? 0 : 1;
        if(ident_sym->type == SYM_TYPE_VAR && ident_sym->belong == cur_scope
           && !ident_sym->initialized) {
            printf("Error row %d : '%s' used without initialized\n",
                   w->row, identname(w));
            return 1;
        }

        if(ident_sym->type == SYM_TYPE_CONST)
            ident_sym = symbol_create_number(ident_sym->const_value);

        if(err)
            return err;
        if(type == MIDOP_ASSIGN)
            mid_statement_insert(cur_scope->ss, MIDOP_ASSIGN, sym, ident_sym);
        else
            mid_statement_insert(cur_scope->ss, type, sym, ident_sym);
        return 0;
    case VT_NUM:
        ident_sym = symbol_create_number(w->ivalue);
        if(type == MIDOP_ASSIGN)
            mid_statement_insert(cur_scope->ss, MIDOP_ASSIGN, sym, ident_sym);
        else
            mid_statement_insert(cur_scope->ss, type, sym, ident_sym);
        (*idx)++;
        return 0;
    case VT_LBRACKETS:
        (*idx)++;
        if(type == MIDOP_ASSIGN) {
            check_subroutine_ret(err, parse_expression, wt, idx, sym);
        } else {
            value = symbol_create_temp();
            mid_statement_insert(cur_scope->ss, MIDOP_GETTEMP, value, NULL);

            check_subroutine_ret(err, parse_expression, wt, idx, value);
            mid_statement_insert(cur_scope->ss, type, sym, value);
            mid_statement_insert(cur_scope->ss, MIDOP_DROPTEMP, value, NULL);
        }
        check_type_inc_idx(wt, idx, VT_RBRACKETS, "')'", 1);
        return 0;
    default:
        print_err_expect("identifier, number, '('", &wt->words[*idx]);
        return 1;
    }
}