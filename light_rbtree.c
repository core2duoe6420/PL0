#include "light_rbtree.h"
#include <stdio.h>
#include <stdlib.h>

#define CMP_ELE_FUNC 1
#define CMP_KEY_FUNC 0

#define COLOR_RED 1
#define COLOR_BLACK 0

static int left_rotate(light_rbtree rbtree, light_rbtree_node x)
{
    //没有右子树，不能左旋
    if(!x->right)
        return -RBTREE_ENORC;

    light_rbtree_node y;
    y = x->right;
    x->right = y->left;
    if(y->left)
        y->left->father = x;
    y->father = x->father;
    if(x->father) {
        if(x == x->father->left)
            x->father->left = y;
        else if(x == x->father->right)
            x->father->right = y;
    } else {
        rbtree->root = y;
    }
    y->left = x;
    x->father = y;
    return 0;
}

static int right_rotate(light_rbtree rbtree, light_rbtree_node x)
{
    //没有左子树，不能右旋
    if(!x->left)
        return -RBTREE_ENOLC;

    light_rbtree_node y;
    y = x->left;
    x->left = y->right;
    if(y->right)
        y->right->father = x;
    y->father = x->father;
    if(x->father) {
        if(x == x->father->left)
            x->father->left = y;
        else if(x == x->father->right)
            x->father->right = y;
    } else {
        rbtree->root = y;
    }
    y->right = x;
    x->father = y;
    return 0;
}

light_rbtree light_rbtree_create(size_t ele_size,
                                 light_rbtree_cmp_func cmp_by_ele,
                                 light_rbtree_cmp_func cmp_by_key,
                                 light_rbtree_link_func link,
                                 light_rbtree_tostring_func to_string)
{
    if(!cmp_by_ele || !cmp_by_key)
        return NULL;

    light_rbtree rbtree;
    rbtree = (light_rbtree)malloc(sizeof(struct __tree));
    if(!rbtree)
        return NULL;
    rbtree->cmp_by_ele = cmp_by_ele;
    rbtree->cmp_by_key = cmp_by_key;
    rbtree->link = link;
    rbtree->to_string = to_string;
    rbtree->element_size = ele_size;
    rbtree->root = NULL;
    return rbtree;
}

char * light_rbtree_print_word(light_rbtree rbtree, void * key, char * buf, int max)
{
    if(!rbtree->to_string)
        return NULL;

    light_rbtree_node node = light_rbtree_get_node(rbtree, key);
    rbtree->to_string(node->element, buf, max);
    return buf;
}

static light_rbtree_node search(light_rbtree rbtree, void * ele, int func_type)
{
    if(!rbtree)
        return NULL;
    light_rbtree_node node;
    light_rbtree_cmp_func func;
    func = (func_type == CMP_ELE_FUNC ? rbtree->cmp_by_ele : rbtree->cmp_by_key);
    node = rbtree->root;
    while(node) {
        int cmpret = func(node->element, ele);
        if(cmpret == 0)
            break;
        else if(cmpret > 0)
            node = node->left;
        else if(cmpret < 0)
            node = node->right;
    }
    return node;
}

void * light_rbtree_get_ele(light_rbtree rbtree, void * key)
{
    light_rbtree_node node;
    node = search(rbtree, key, CMP_KEY_FUNC);
    if(node)
        return node->element;
    return NULL;
}

light_rbtree_node light_rbtree_get_node(light_rbtree rbtree, void * key)
{
    return search(rbtree, key, CMP_KEY_FUNC);
}

static light_rbtree_node get_minimum_node(light_rbtree_node node)
{
    while(node->left)
        node = node->left;
    return node;
}

static light_rbtree_node get_maximum_node(light_rbtree_node node)
{
    while(node->right)
        node = node->right;
    return node;
}

static light_rbtree_node get_successor(light_rbtree_node node)
{
    if(node->right)
        return get_minimum_node(node->right);

    light_rbtree_node tmp;
    tmp = node->father;
    while(tmp && node == tmp->right) {
        node = tmp;
        tmp = tmp->father;
    }
    return tmp;
}

static light_rbtree_node get_predecessor(light_rbtree_node node)
{
    if(node->left)
        return get_maximum_node(node->left);
    light_rbtree_node tmp;
    tmp = node->father;
    while(tmp && node == tmp->left) {
        node = tmp;
        tmp = tmp->father;
    }
    return tmp;
}

static inline int is_red(light_rbtree_node node)
{
    if(node)
        return node->color == COLOR_RED;
    //NULL结点是黑的
    return 0;
}

//算法导论P167
static void insert_fixup(light_rbtree rbtree, light_rbtree_node node_tofix)
{
    light_rbtree_node uncle;
    //循环条件保证了父节点不是根，因此存在祖父结点
    while(is_red(node_tofix->father)) {
        if(node_tofix->father == node_tofix->father->father->left) {
            //第一部分 父结点是祖父结点的左孩子
            uncle = node_tofix->father->father->right;

            if(is_red(uncle)) {
                /* case 1:叔叔是红色的
                 *将父节点和叔叔结点变为黑色，祖父结点变为红色
                 *然后将node_tofix上移到祖父结点
                 */
                node_tofix->father->color = COLOR_BLACK;
                uncle->color = COLOR_BLACK;
                node_tofix->father->father->color = COLOR_RED;
                node_tofix = node_tofix->father->father;
            } else if(node_tofix == node_tofix->father->right) {
                /* case 2:node_tofix是父节点的右孩子
                 * 通过对node_tofix的父节点左旋变为case 3
                 */
                node_tofix = node_tofix->father;
                left_rotate(rbtree, node_tofix);
            } else if(node_tofix == node_tofix->father->left) {
                /* case 3:node_tofix是父节点的左孩子
                 * 通过对祖父结点右旋并改变颜色修正冲突
                 */
                node_tofix->father->color = COLOR_BLACK;
                node_tofix->father->father->color = COLOR_RED;
                right_rotate(rbtree, node_tofix->father->father);
            }
        } else {
            //第二部分 父节点是祖父结点的右孩子
            uncle = node_tofix->father->father->left;

            if(is_red(uncle)) {
                /* case 4:处理方式与case 1相同 */
                node_tofix->father->color = COLOR_BLACK;
                uncle->color = COLOR_BLACK;
                node_tofix->father->father->color = COLOR_RED;
                node_tofix = node_tofix->father->father;
            } else if(node_tofix == node_tofix->father->left) {
                /* case 5:node_tofix是父节点的左孩子
                 * 通过对node_tofix的父节点右旋变为case 6
                 */
                node_tofix = node_tofix->father;
                right_rotate(rbtree, node_tofix);
            } else if(node_tofix == node_tofix->father->right) {
                /* case 6:node_tofix是父节点的右孩子
                 * 通过对祖父结点左旋并改变颜色修正冲突
                 */
                node_tofix->father->color = COLOR_BLACK;
                node_tofix->father->father->color = COLOR_RED;
                left_rotate(rbtree, node_tofix->father->father);
            }
        }
    }
    rbtree->root->color = COLOR_BLACK;
}

static int insert(light_rbtree rbtree, light_rbtree_node node_to_insert)
{
    if(!rbtree || !node_to_insert)
        return -RBTREE_ENULL;

    int cmpret;
    light_rbtree_node node, father = NULL;
    node = rbtree->root;
    while(node) {
        father = node;
        cmpret = rbtree->cmp_by_ele(node->element, node_to_insert->element);
        if(cmpret < 0) {
            node = node->right;
        } else if(cmpret > 0) {
            node = node->left;
        } else if(cmpret == 0) {
            if(!rbtree->link)
                return -RBTREE_ENOLINK;
            rbtree->link(node->element, node_to_insert->element);
            //元素插入到链表后不需要树节点了
            free(node_to_insert);
            return 0;
        }
    }
    node_to_insert->father = father;
    if(!father) {
        rbtree->root = node_to_insert;
    } else {
        if(cmpret < 0)
            father->right = node_to_insert;
        else if(cmpret > 0)
            father->left = node_to_insert;
    }
    insert_fixup(rbtree, node_to_insert);
    return 0;
}

static light_rbtree_node light_rbtree_node_create(void * ele)
{
    light_rbtree_node node;
    node = (light_rbtree_node)malloc(sizeof(struct __treenode));
    if(!node)
        return NULL;
    node->element = ele;
    node->father = NULL;
    node->left = NULL;
    node->right = NULL;
    node->color = COLOR_RED;
    return node;
}

int light_rbtree_insert(light_rbtree rbtree, void * ele)
{
    light_rbtree_node node;
    node = light_rbtree_node_create(ele);
    if(node)
        return insert(rbtree, node);
    else
        return -RBTREE_ENULL;
}

static void free_tree(light_rbtree_node node)
{
    if(node->left)
        free_tree(node->left);
    if(node->right)
        free_tree(node->right);
    free(node);
}

int light_rbtree_delete(light_rbtree rbtree)
{
    if(!rbtree)
        return -RBTREE_ENULL;
    free_tree(rbtree->root);
    free(rbtree);
    return 0;
}

int light_rbtree_left_rotate(light_rbtree rbtree, void * key)
{
    light_rbtree_node node;
    node = search(rbtree, key, CMP_KEY_FUNC);
    return left_rotate(rbtree, node);
}

int light_rbtree_right_rotate(light_rbtree rbtree, void * key)
{
    light_rbtree_node node;
    node = search(rbtree, key, CMP_KEY_FUNC);
    return right_rotate(rbtree, node);
}