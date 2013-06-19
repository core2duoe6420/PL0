#ifndef __LIGHTSTACK_H
#define __LIGHTSTACK_H

#ifdef __LINUX__
#include <stddef.h>
#endif

struct __stack {
    int max;
    int top;
    int type;
    int auto_inc;

    size_t element_size;
    int block_size;
    struct buffer_block * block;
    struct buffer_block * block_cache;

    size_t alloc_mem;
    size_t max_mem;
};

/* 栈类型。所有对栈的操作都应通过该类型。
 * 不要访问结构中的成员。
 */
typedef struct __stack * light_stack;

struct buffer_block {
    struct buffer_block * next;
    light_stack stack;
    int block_size;
    int block_top;
    void * buffer;
    size_t alloc_mem;
};

/* 创建一个栈结构。
 * @type：STACK_PTR表明栈中存放的是指针，
          STACK_INSTANCE表明栈中存放的是完整的数据。
 * @stack_max：STACK_CAPACITY_AUTO(0)表明栈自动增长，不为0则指定栈的最大长度。
 * @ele_size：栈内元素的大小，如果类型为STACK_PTR，则固定为指针的大小。
 * @block_size：每次栈增长的长度。小于等于0则为默认值128。
 * @return：返回指向栈类型。出现任何错误返回NULL。首次分配的内存数量不得超过100M。
 */
light_stack light_stack_create(int type, int stack_max, size_t ele_size, int block_size);
/* 往栈中压入一个数据。
 * @ele：指向数据的指针，如果类型为STACK_INSTANCE，将完整复制ele指向的数据，
         如果类型为STACK_PTR，则复制ele指针本身的值。
 * @return：成功返回0，参数错误返回-NULL，栈不是自动增长且已满返回-EFULL，
            增长栈时内存分配错误返回-EALLOC，栈已超出最大内存返回-EOOM。
 */
int light_stack_push(light_stack stack, void * ele);
/* 弹出栈顶数据。
 * @ele：数据将被写入ele指向的地址，如果类型为STACK_PTR，应该传入指针变量的地址
         如果类型为STACK_INSTANCE,传入指针本身即可。
 * @return：成功返回0，参数错误返回-NULL，栈空返回-EEMPTY。
 */
int light_stack_pop(light_stack stack, void * ele);
/* 读取栈顶数据，不弹出。
 * @ele：数据将被写入ele指向的地址，如果类型为STACK_PTR，应该传入指针变量的地址，
         如果类型为STACK_INSTANCE,传入指针本身即可。
 * @return：成功返回0，参数错误返回-NULL，栈空返回-EEMPTY。
 */
int light_stack_peek(light_stack stack, void * ele);
/* 获得栈内元素数量。
 * @return：正数为栈内元素数量，参数错误-ENULL。
 */
int light_stack_count(light_stack stack);
/* 栈是否为空
 * @return：1表示栈空，0表示栈非空，参数错误-ENULL。
 */
int light_stack_isempty(light_stack stack);
/* 删除一个栈，释放所有内存。传入的指针会被修改为NULL。
 * 如果栈类型为TYPE_PTR，栈内元素指向的数据不会释放。
 * @return：成功返回0。
 */
int light_stack_delete(light_stack * stack);
/* 栈是否满。
 * 类型为TYPE_INSTANCE的栈永远返回0。
 * @return：1表示栈满，0表示栈不满，参数错误-ENULL。
 */
int light_stack_isfull(light_stack stack);
/* 设置栈自动增长大小。
 * @return：成功返回0。参数错误返回-ENULL或-EARGUE。增长数量超出内存允许范围返回-EOOM。
 */
int light_stack_set_block_size(light_stack stack, int block_size);
/* 设置栈允许占用的估计最大内存，单位为字节，默认为300M,最大1.5G。
 * 注意在创建栈时申请的内存不受限制。该值仅能保证栈在超出设定数值后不会再增长，
 * 不能保证占用的内存一定不会超过该值。
 * @return：成功返回0。参数错误返回-ENULL,超出范围返回-EOOM。
 */
int light_stack_set_max_mem(light_stack stack, size_t max_mem);

#define STACK_EALLOC 100
#define STACK_ENULL 101
#define STACK_EEMPTY 102
#define STACK_EFULL 103
#define STACK_EARGUE 104
#define STACK_EOOM 105

#define STACK_CAPACITY_AUTO 0

#define STACK_PTR 1
#define STACK_INSTANCE 2
#define STACK_UNDEFINED 3

#endif