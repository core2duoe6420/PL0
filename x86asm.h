#ifndef __X86ASM_H
#define __X86ASM_H

#define INT_LENGTH 4

struct x86reg {
    char * name;
    struct symbol * sym;
    int lock;
};

//�Է���Ϊ��ַ��������������ȫ�ֱ���
#define ADDR_SYMBOL 1
//�Ĵ������Ѱַ
#define ADDR_REG_RELATIVE 2
//�Ĵ���Ѱַ
#define ADDR_REG 3
//������
#define ADDR_IMM 4
//NASM�﷨lea����Ҫ�ֳ�
#define ADDR_SYMBOL_NO_DWORD 5
#define ADDR_REG_RELATIVE_NO_DWORD 6

struct address {
    //typeָ��symbol����Ҫ��Ѱַ��ʽ
    int type;
    struct symbol * sym;
    struct x86reg * reg;
    //base+offset����ջ�ڵ�ַ
    struct x86reg * base;
    int offset;
    int dirty;
};

#define X86_MOV 0
#define X86_ADD 1
#define X86_SUB 2
#define X86_MUL 3
#define X86_DIV 4
#define X86_NEG 5
#define X86_JMP 6
#define X86_JG 7
#define X86_JGE 8
#define X86_JL 9
#define X86_JLE 10
#define X86_JE 11
#define X86_JNE 12
#define X86_CMP 13
#define X86_CALL 14
#define X86_RET 15
#define X86_PUSH 16
#define X86_POP 17
#define X86_LEAVE 18
#define X86_LEA 19
#define X86_PUSHAD 20
#define X86_POPAD 21
//Ϊ�˷�����
#define X86_MOD 10000


#define PUSHAD_REGS 7
struct x86_instruction {
    int type;
    char * format;
};

int generate_x86asm(char * output);
void initial_address(struct address * addr, int type, struct x86reg * reg,
                     struct x86reg * base, int offset);
#endif