#include "sym_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SYM_INCREMENT 32

struct scope * global_scope;
struct scope * temp_scope;

static int symbol_cmp_ele(void * sym_a, void * sym_b)
{
    return ((struct symbol *)sym_a)->hash -
           ((struct symbol *)sym_b)->hash;
}

static int symbol_cmp_key(void * sym, void * key)
{
    return ((struct symbol *)sym)->hash -
           *(unsigned int *)key;
}

static int symbol_link(void * sym_a, void * sym_b)
{
    struct symbol * ptr_a = (struct symbol *)sym_a;
    struct symbol * ptr_b = (struct symbol *)sym_b;
    ptr_b->tree_link = ptr_a->tree_link;
    ptr_a->tree_link = ptr_b;
    return 0;
}

//仅分配空间赋初值
static struct sym_block * sym_block_create(int count) {
    struct sym_block * block;
    block = (struct sym_block *)malloc(sizeof(struct sym_block));
    if(!block)
        return NULL;
    block->next = NULL;
    block->count = 0;
    block->max = count;
    block->symbols = (struct symbol *)malloc(sizeof(struct symbol)*count);
    if(!block->symbols) {
        free(block);
        return NULL;
    }
    return block;
}

//分配空间赋初值 对外接口为scope_new
static struct scope * scope_create(struct scope * father, char * name) {
    struct scope * scp;
    scp = (struct scope *)malloc(sizeof(struct scope));
    if(!scp)
        return NULL;
    scp->name = name;
    scp->child = NULL;
    scp->sibling = NULL;
    scp->father = father;
    if(father) {
        scp->sibling = father->child;
        father->child = scp;
    }
    scp->sym_tree = light_rbtree_create(sizeof(struct scope),
                                        symbol_cmp_ele,
                                        symbol_cmp_key,
                                        symbol_link,
                                        NULL);
    if(!scp->sym_tree) {
        free(scp);
        return NULL;
    }

    scp->sym_count = 0;
    scp->sym_max = SYM_INCREMENT;
    scp->block = sym_block_create(SYM_INCREMENT);
    if(!scp->block) {
        light_rbtree_delete(scp->sym_tree);
        free(scp);
        return NULL;
    }
    scp->ss = statements_set_create();
    if(!scp->ss) {
        light_rbtree_delete(scp->sym_tree);
        free(scp->block);
        free(scp);
        return NULL;
    }

    return scp;
}

//创建作用域的对外接口，主要工作为分配器，区分局部过程和全局作用域
struct scope * scope_new(struct scope * father, struct symbol * proc_sym) {
    struct scope * new_scp;
    if(father) {
        new_scp = scope_create(father, proc_sym->name);
        proc_sym->proc = new_scp;
    } else {
        new_scp = scope_create(NULL, "global");
    }
    return new_scp;
}

//从block缓存中获得一个新的struct symbol实例。如果block满，会分配一个新的block
static struct symbol * get_symbol_instance(struct scope * scp) {
    struct sym_block * block = scp->block;
    while(block->next)
        block = block->next;

    if(block->count >= block->max) {
        block->next = sym_block_create(SYM_INCREMENT);
        if(!block->next)
            return NULL;
        scp->sym_max += SYM_INCREMENT;
        block = block->next;
    }
    return block->symbols + block->count++;
}

//向作用域添加一个符号。会调用get_symbol_instance获得一个新的实例
//err用于返回错误信息，即使有同名符号依旧添加，但会返回1
struct symbol * scope_add_symbol(struct scope * scp, char * name, int type, int * err) {
    if(!scp)
        return NULL;

    struct symbol * sym;
    unsigned int hash;
    hash = get_string_hash(name);
    //存在同名符号?
    sym = scope_search_symbol(scp, hash);
    while(sym) {
        if(strcmp(sym->name, name) == 0) {
            if(err)
                *err = 1;
            break;
        }
        sym = sym->tree_link;
    }

    sym = get_symbol_instance(scp);
    if(!sym)
        return NULL;

    scp->sym_count++;
    sym->belong = scp;
    strncpy(sym->name, name, sizeof(sym->name));
    sym->hash = hash;
    sym->tree_link = NULL;
    sym->type = type;
    sym->proc = NULL;
    initial_address(&sym->addr, 0, NULL, NULL, 0);
    sym->addr.sym = sym;
    sym->initialized = 0;
    light_rbtree_insert(scp->sym_tree, sym);
    return sym;
}

//将符号表看成线性表，通过idx下标访问，通常用于顺序遍历
struct symbol * scope_get_symbol(struct scope * scp, int idx) {
    if(!scp || idx < 0 || idx >= scp->sym_count)
        return NULL;

    struct sym_block * block = scp->block;
    int block_id;
    block_id = idx / SYM_INCREMENT;
    idx = idx % SYM_INCREMENT;
    while(block_id--)
        block = block->next;

    return block->symbols + idx;
}

//通过hash搜索一个作用域，散列冲突的符号会通过tree_link链在链表上
struct symbol * scope_search_symbol(struct scope * scp, unsigned int hash) {
    struct symbol * sym;
    sym = (struct symbol *)light_rbtree_get_ele(scp->sym_tree, &hash);
    return sym;
}

struct symbol * scope_get_symbol_by_name(struct scope * scp, char * name) {
    struct symbol * sym;
    unsigned int hash = get_string_hash(name);
    sym = scope_search_symbol(scp, hash);
    while(sym) {
        if(strcmp(sym->name, name) == 0)
            return sym;
        sym = sym->tree_link;
    }
    return NULL;
}

//查找一个标识符是否在符号表中，会自底向上搜索至全局符号表
struct symbol * scope_get_symbol_by_word(struct scope * scp, struct word * w) {
    struct symbol * sym;
    while(scp) {
        //自底向上搜索
        sym = scope_search_symbol(scp, w->ident_hash);
        while(sym) {
            //散列冲突？
            if(strcmp(sym->name, identname(w)) == 0)
                return sym;
            sym = sym->tree_link;
        }
        scp = scp->father;
    }
    return NULL;
}

//释放一个block，没有调整任何数据结构，仅可在释放整个作用域结构体时调用
static inline void sym_block_free(struct sym_block * block)
{
    free(block->symbols);
    free(block);
}

//释放所有的作用域
void scope_delete_tree(struct scope * scp)
{
    if(!scp)
        return;
    scope_delete_tree(scp->child);
    scope_delete_tree(scp->sibling);
    scope_delete(scp);
}

//释放单个作用域
int scope_delete(struct scope * scp)
{
    if(!scp)
        return 1;

    struct sym_block * block, * next;
    block = scp->block;
    while(block) {
        next = block->next;
        sym_block_free(block);
        block = next;
    }
    statements_set_drop(scp->ss);
    free(scp);
    return 0;
}

//打印全部作用域
void scope_print_tree(struct scope * father)
{
    static int idx = 0;
    int ret;
    ret = scope_print(father, idx);
    idx += ret;
    if(father->child)
        scope_print_tree(father->child);
    if(father->sibling)
        scope_print_tree(father->sibling);

}

//打印单个作用域
int scope_print(struct scope * scp, int idx)
{
    struct symbol * sym;
    char * type;
    printf("Symbol table for %s\n", scp->name);
    for(int i = 0; i < scp->sym_count; i++) {
        sym = scope_get_symbol(scp, i);
        switch(sym->type) {
        case SYM_TYPE_PROC:
            type = "procedure";
            break;
        case SYM_TYPE_VAR:
            type = "variable";
            break;
        case SYM_TYPE_CONST:
            type = "const";
            break;
        }
        printf("%d\t\%15s%12s\t%s\n", idx++, sym->name,
               type, sym->belong == global_scope ? "global" : "local");
    }
    printf("\n");
    return scp->sym_count;
}

//初始化全局作用域
void initial_scope()
{
    global_scope = scope_new(NULL, NULL);
    temp_scope = scope_new(NULL, NULL);
}

static struct symbol * symbol_generate(int type, int value, char * str) {
    static int label_count = 0, temp_count = 0;
    int * count;
    char * outname;
    if(type == SYM_TYPE_LABEL) {
        count = &label_count;
        outname = "LBL%d";
    } else if(type == SYM_TYPE_TEMP) {
        count = &temp_count;
        outname = "TMP%d";
    } else if(type == SYM_TYPE_NUMBER) {
        count = &value;
        outname = "%d";
    } else if(type == SYM_TYPE_STRING) {
        count = &value;
        outname = str;
    } else {
        return NULL;
    }

    char labelname[32];
    sprintf(labelname, outname, *count);
    (*count)++;

    struct symbol * sym;
    sym = scope_add_symbol(temp_scope, labelname, type, NULL);
    return sym;
}
extern struct scope * cur_scope;
struct symbol * symbol_create_label() {
    struct symbol * sym;
    sym = symbol_generate(SYM_TYPE_LABEL, 0, NULL);
    mid_statement_insert(cur_scope->ss, MIDOP_CREATE_LABEL, sym, NULL);
    return sym;
}

struct symbol * symbol_create_temp() {
    return symbol_generate(SYM_TYPE_TEMP, 0, NULL);
}

struct symbol * symbol_create_number(int n) {
    struct symbol * sym;
    sym = symbol_generate(SYM_TYPE_NUMBER, n, NULL);
    sym->addr.type = ADDR_IMM;
    sym->addr.offset = n;
    return sym;
}

struct symbol * symbol_create_string(char * str) {
    return symbol_generate(SYM_TYPE_STRING, 0, str);
}