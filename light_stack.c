#include "light_stack.h"
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
#include <stdio.h>
#endif

#define DEFAULT_BLOCK_SIZE 128
//����ʱ����������Ĵ�С
#define DEFAULT_MAX_MEM (300*1024*1024)
//����ջʱ������ڴ��С
#define DEFAULT_CREATE_MEM (100*1024*1024)
#define ABSOLUTE_MAX_MEM (1500*1024*1024)

static inline void add_mem_allocated(light_stack stack, size_t mem)
{
    stack->alloc_mem += mem;
}

static inline void minus_mem_allocated(light_stack stack, size_t mem)
{
    stack->alloc_mem -= mem;
}

static struct buffer_block * buffer_block_create(size_t ele_size, int block_size) {
    struct buffer_block * block;
    block = (struct buffer_block *)malloc(sizeof(struct buffer_block));
    if(!block)
        return NULL;
    block->block_size = block_size;
    block->block_top = 0;
    block->stack = NULL;
    block->next = NULL;
    block->buffer = malloc(ele_size * block_size);
    if(!block->buffer) {
        free(block);
        return NULL;
    }

    //����������ڴ�����������ջ��ͳ�ơ�
    block->alloc_mem = sizeof(struct buffer_block) + ele_size * block_size;

#ifdef DEBUG
    printf("alloc:%p\n", block);
#endif

    return block;
}

static void buffer_block_insert_to_stack(struct buffer_block * block, light_stack stack)
{
    block->stack = stack;
    block->next = stack->block;
    stack->block = block;
    //����������ջ�л��Զ�����ջ��max
    stack->max += block->block_size;
}

static struct buffer_block * buffer_block_delete(struct buffer_block * block) {
    struct buffer_block * next;
    next = block->next;
    free(block->buffer);
    free(block);

#ifdef DEBUG
    printf("free %p\n", block);
#endif

    return next;
}

static int buffer_block_push(struct buffer_block * block, void * ele, size_t ele_size)
{
    if(block->block_top >= block->block_size)
        return -STACK_EFULL;

    memcpy((char*)block->buffer + block->block_top * ele_size, ele, ele_size);
    block->block_top++;
    return 0;
}

static int buffer_block_peek(struct buffer_block * block, void * ele, size_t ele_size)
{
    memcpy(ele, (char*)block->buffer + (block->block_top - 1) * ele_size, ele_size);
    return 0;
}

static int buffer_block_pop(struct buffer_block * block, void * ele, size_t ele_size)
{
    int ret = buffer_block_peek(block, ele, ele_size);
    if(!ret)
        block->block_top--;
    return ret;
}

static int check_block_empty(light_stack stack)
{
    struct buffer_block * block;
    block = stack->block;

    if(block->block_top)
        return 0;

    //ɾ��һ����ʱ���������ͷ�������ֹ��pop����push�˷������ڴ��ʱ�䡣
    //�ϴα�ɾ���Ŀ鱣����block_cache�У����block_cache��0����ô��ʱ�ͷ�����
    if(stack->block_cache) {
        minus_mem_allocated(stack, block->alloc_mem);
        buffer_block_delete(stack->block_cache);
    }

    stack->block_cache = block;
    stack->block = block->next;
    //��ջ��ɾ��һ�������Ҫ��ȥmax
    stack->max -= block->block_size;
    block->next = NULL;
    return 1;
}

static int check_block_full(light_stack stack)
{
    struct buffer_block * block;
    block = stack->block;
    if(block->block_top < block->block_size)
        return 0;

    //���ȼ���Ƿ��б����Ļ���顣
    if(stack->block_cache) {
        buffer_block_insert_to_stack(stack->block_cache, stack);
        stack->block_cache = NULL;
    } else {
        //û�л���飬��������һ���ڴ档
        //�Ƿ��Ѿ���������ڴ����ƣ�
        if(stack->alloc_mem >= stack->max_mem)
            return -STACK_EOOM;
        struct buffer_block * block;
        block = buffer_block_create(stack->element_size, stack->block_size);
        if(!block)
            return -STACK_EALLOC;
        add_mem_allocated(stack, block->alloc_mem);
        buffer_block_insert_to_stack(block, stack);
    }
    return 0;
}

light_stack light_stack_create(int type, int stack_max, size_t ele_size, int block_size)
{
    if(type >= STACK_UNDEFINED)
        type = STACK_INSTANCE;

    if(type == STACK_PTR)
        ele_size = sizeof(void *);

    if(stack_max < 0)
        stack_max = 0;

    if(block_size <= 0)
        block_size = DEFAULT_BLOCK_SIZE;

    int auto_inc = stack_max ? 0 : 1;
    stack_max = stack_max ? stack_max : block_size;

    //�ڴ�ռ��̫�󣬲�������������
    if(stack_max * ele_size >= DEFAULT_CREATE_MEM)
        return NULL;

    light_stack tmp;
    tmp = (light_stack)malloc(sizeof(struct __stack));
    if(!tmp)
        return NULL;

    tmp->element_size = ele_size;
    tmp->top = 0;
    tmp->max = 0;
    tmp->auto_inc = auto_inc;
    tmp->type = type;
    tmp->block = NULL;
    tmp->block_cache = NULL;
    tmp->block_size = block_size;
    tmp->alloc_mem = 0;
    tmp->max_mem = DEFAULT_MAX_MEM;

    add_mem_allocated(tmp, sizeof(struct __stack));

    struct buffer_block * block;
    block = buffer_block_create(ele_size, stack_max);
    if(!block) {
        free(tmp);
        return NULL;
    }

    add_mem_allocated(tmp, block->alloc_mem);
    buffer_block_insert_to_stack(block, tmp);

    return tmp;
}

int light_stack_push(light_stack stack, void * ele)
{
    if(!stack)
        return -STACK_ENULL;
    if(stack->top >= stack->max) {
        if(stack->auto_inc) {
            int ret = check_block_full(stack);
            if(ret)
                return ret;
        } else {
            return -STACK_EFULL;
        }
    }

    void * ptr;
    //STACK_PTR���͵�ջ��ŵ���ָ�뱾���ֵ��
    if(stack->type == STACK_PTR)
        ptr = &ele;
    else
        ptr = ele;

    buffer_block_push(stack->block, ptr, stack->element_size);
    stack->top++;

    return 0;
}

int light_stack_peek(light_stack stack, void * ele)
{
    if(!stack)
        return -STACK_ENULL;
    if(!stack->top)
        return -STACK_EEMPTY;
    buffer_block_peek(stack->block, ele, stack->element_size);
    return 0;
}

int light_stack_pop(light_stack stack, void * ele)
{
    if(!stack)
        return -STACK_ENULL;
    if(!stack->top)
        return -STACK_EEMPTY;

    buffer_block_pop(stack->block, ele, stack->element_size);
    stack->top--;
    //ֻ�еڶ��黺�����Ԫ������Ϊ0ʱ��Ҫ��������顣
    //����ǵ�һ�黺��飬��ô����Ҫ��ʲô���顣
    if(stack->top)
        check_block_empty(stack);

    return 0;
}

int light_stack_count(light_stack stack)
{
    if(!stack)
        return -STACK_ENULL;
    return stack->top;
}

int light_stack_isempty(light_stack stack)
{
    if(!stack)
        return -STACK_ENULL;
    return stack->top == 0;
}

int light_stack_isfull(light_stack stack)
{
    if(!stack)
        return -STACK_ENULL;
    return (!stack->auto_inc) && stack->top == stack->max;
}

int light_stack_delete(light_stack * stack)
{
    struct buffer_block * block;
    block = (*stack)->block;
    while(block = buffer_block_delete(block))
        ;
    //�����Ļ���鲻�������У������ͷš�
    if((*stack)->block_cache)
        buffer_block_delete((*stack)->block_cache);
    free(*stack);
    *stack = NULL;
    return 0;
}

int light_stack_set_block_size(light_stack stack, int block_size)
{
    if(!stack)
        return -STACK_ENULL;
    if(block_size <= 0)
        return -STACK_EARGUE;
    if(block_size * stack->element_size > ABSOLUTE_MAX_MEM)
        return -STACK_EOOM;

    stack->block_size = block_size;
    return 0;
}

int light_stack_set_max_mem(light_stack stack, size_t max_mem)
{
    if(!stack)
        return -STACK_ENULL;
    if(max_mem > ABSOLUTE_MAX_MEM)
        return -STACK_EOOM;
    stack->max_mem = max_mem;
    return 0;
}