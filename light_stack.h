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

/* ջ���͡����ж�ջ�Ĳ�����Ӧͨ�������͡�
 * ��Ҫ���ʽṹ�еĳ�Ա��
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

/* ����һ��ջ�ṹ��
 * @type��STACK_PTR����ջ�д�ŵ���ָ�룬
          STACK_INSTANCE����ջ�д�ŵ������������ݡ�
 * @stack_max��STACK_CAPACITY_AUTO(0)����ջ�Զ���������Ϊ0��ָ��ջ����󳤶ȡ�
 * @ele_size��ջ��Ԫ�صĴ�С���������ΪSTACK_PTR����̶�Ϊָ��Ĵ�С��
 * @block_size��ÿ��ջ�����ĳ��ȡ�С�ڵ���0��ΪĬ��ֵ128��
 * @return������ָ��ջ���͡������κδ��󷵻�NULL���״η�����ڴ��������ó���100M��
 */
light_stack light_stack_create(int type, int stack_max, size_t ele_size, int block_size);
/* ��ջ��ѹ��һ�����ݡ�
 * @ele��ָ�����ݵ�ָ�룬�������ΪSTACK_INSTANCE������������eleָ������ݣ�
         �������ΪSTACK_PTR������eleָ�뱾���ֵ��
 * @return���ɹ�����0���������󷵻�-NULL��ջ�����Զ���������������-EFULL��
            ����ջʱ�ڴ������󷵻�-EALLOC��ջ�ѳ�������ڴ淵��-EOOM��
 */
int light_stack_push(light_stack stack, void * ele);
/* ����ջ�����ݡ�
 * @ele�����ݽ���д��eleָ��ĵ�ַ���������ΪSTACK_PTR��Ӧ�ô���ָ������ĵ�ַ
         �������ΪSTACK_INSTANCE,����ָ�뱾���ɡ�
 * @return���ɹ�����0���������󷵻�-NULL��ջ�շ���-EEMPTY��
 */
int light_stack_pop(light_stack stack, void * ele);
/* ��ȡջ�����ݣ���������
 * @ele�����ݽ���д��eleָ��ĵ�ַ���������ΪSTACK_PTR��Ӧ�ô���ָ������ĵ�ַ��
         �������ΪSTACK_INSTANCE,����ָ�뱾���ɡ�
 * @return���ɹ�����0���������󷵻�-NULL��ջ�շ���-EEMPTY��
 */
int light_stack_peek(light_stack stack, void * ele);
/* ���ջ��Ԫ��������
 * @return������Ϊջ��Ԫ����������������-ENULL��
 */
int light_stack_count(light_stack stack);
/* ջ�Ƿ�Ϊ��
 * @return��1��ʾջ�գ�0��ʾջ�ǿգ���������-ENULL��
 */
int light_stack_isempty(light_stack stack);
/* ɾ��һ��ջ���ͷ������ڴ档�����ָ��ᱻ�޸�ΪNULL��
 * ���ջ����ΪTYPE_PTR��ջ��Ԫ��ָ������ݲ����ͷš�
 * @return���ɹ�����0��
 */
int light_stack_delete(light_stack * stack);
/* ջ�Ƿ�����
 * ����ΪTYPE_INSTANCE��ջ��Զ����0��
 * @return��1��ʾջ����0��ʾջ��������������-ENULL��
 */
int light_stack_isfull(light_stack stack);
/* ����ջ�Զ�������С��
 * @return���ɹ�����0���������󷵻�-ENULL��-EARGUE���������������ڴ�����Χ����-EOOM��
 */
int light_stack_set_block_size(light_stack stack, int block_size);
/* ����ջ����ռ�õĹ�������ڴ棬��λΪ�ֽڣ�Ĭ��Ϊ300M,���1.5G��
 * ע���ڴ���ջʱ������ڴ治�����ơ���ֵ���ܱ�֤ջ�ڳ����趨��ֵ�󲻻���������
 * ���ܱ�֤ռ�õ��ڴ�һ�����ᳬ����ֵ��
 * @return���ɹ�����0���������󷵻�-ENULL,������Χ����-EOOM��
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