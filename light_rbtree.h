#ifndef __LIGHTRBTREE_H
#define __LIGHTRBTREE_H

#ifdef __LINUX__
#include <stddef.h>
#endif

/* 比较两个元素。
 * 返回0表示相等，正数表示ele_a大于ele_b，负数表示ele_a小于ele_b。
 */
typedef int (*light_rbtree_cmp_func)(void * elea, void * eleb_or_key);

/* 当两个元素相等时，将元素串在一个链表上
 */
typedef int (*light_rbtree_link_func)(void * prev, void * next);

typedef void (*light_rbtree_tostring_func)(void * ele, char * buf, int max);

struct __treenode {
    void * element;
    struct __treenode * father;
    struct __treenode * left;
    struct __treenode * right;
    int color;
};

struct __tree {
    int node_nr;
    struct __treenode * root;
    size_t element_size;
    //用于元素与元素之间的比较
    light_rbtree_cmp_func cmp_by_ele;
    //用于元素与关键字之间的比较
    light_rbtree_cmp_func cmp_by_key;
    light_rbtree_link_func link;
    light_rbtree_tostring_func to_string;
};

typedef struct __treenode * light_rbtree_node;
typedef struct __tree * light_rbtree;

#define RBTREE_ENULL 100
#define RBTREE_ENOLINK 101
//NO RIGHT CHILD
#define RBTREE_ENORC 102
//NO LEFT CHILD
#define RBTREE_ENOLC 102
int light_rbtree_insert(light_rbtree rbtree, void * ele);
void * light_rbtree_get_ele(light_rbtree rbtree, void * key);
light_rbtree_node light_rbtree_get_node(light_rbtree rbtree, void * key);
char * light_rbtree_print_word(light_rbtree rbtree, void * key, char * buf, int max);
light_rbtree light_rbtree_create(size_t ele_size,
                                 light_rbtree_cmp_func cmp_by_ele,
                                 light_rbtree_cmp_func cmp_by_key,
                                 light_rbtree_link_func link,
                                 light_rbtree_tostring_func to_string);
int light_rbtree_delete(light_rbtree rbtree);
int light_rbtree_left_rotate(light_rbtree rbtree, void * key);
int light_rbtree_right_rotate(light_rbtree rbtree, void * key);
#endif