#ifndef __SYM_TABLE_H
#define __SYM_TABLE_H

#include "lexer.h"
#include "light_rbtree.h"
#include "midcode.h"
#include "x86asm.h"

struct scope {
    char * name;
    //sym_tree用于搜索，block用于遍历
    light_rbtree sym_tree;
    struct sym_block * block;
    int sym_count;
    int sym_max;
    struct scope * father;
    struct scope * child;
    struct scope * sibling;
    struct statements_set * ss;
};

struct sym_block {
    struct symbol * symbols;
    struct sym_block * next;
    int count;
    int max;
};

struct symbol {
    int type;
    char name[ID_MAX_LEN];
    unsigned int hash;
    //所属的作用域
    struct scope * belong;
    //procedure产生的新作用域
    struct scope * proc;
    //红黑树中散列冲突时置于该链表上
    struct symbol * tree_link;
    union {
        //如果type是SYM_TYPE_CONST该字段保存值
        int const_value;
        //如果是SYM_TYPE_VAR，用于语法分析中检查是否初始化
        int initialized;
    };
    struct address addr;
};

#define SYM_TYPE_PROC 1
#define SYM_TYPE_VAR 2
#define SYM_TYPE_CONST 3
#define SYM_TYPE_VAR_CONST 4
#define SYM_TYPE_LABEL 5
#define SYM_TYPE_TEMP 6
#define SYM_TYPE_NUMBER 7
#define SYM_TYPE_STRING 8

struct symbol * scope_add_symbol(struct scope * scp, char * name, int type, int * err);
struct scope * scope_new(struct scope * father, struct symbol * proc_sym);
struct symbol * scope_get_symbol(struct scope * scp, int idx);
struct symbol * scope_search_symbol(struct scope * scp, unsigned int hash);
struct symbol * scope_get_symbol_by_name(struct scope * scp, char * name);
struct symbol * scope_get_symbol_by_word(struct scope * scp, struct word * w);
int scope_delete(struct scope * scp);
int scope_print(struct scope * scp, int idx);
void scope_print_tree(struct scope * father);
void initial_scope();
void scope_delete_tree(struct scope * scp);

struct symbol * symbol_create_label();
struct symbol * symbol_create_temp();
struct symbol * symbol_create_number(int n);
struct symbol * symbol_create_string(char * str);

extern struct scope * global_scope;
extern struct scope * temp_scope;
#endif