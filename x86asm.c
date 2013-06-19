#include "sym_table.h"
#include "x86asm.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#ifndef __LINUX__
#define PRINTF "_printf"
#define SCANF "_scanf"
#define MAIN "_main"
#else
#define PRINTF "printf"
#define SCANF "scanf"
#define MAIN "main"
#endif


FILE * x86output;

#define EAX (&x86regs[0])
#define EBX (&x86regs[1])
#define ECX (&x86regs[2])
#define EDX (&x86regs[3])
#define ESI (&x86regs[4])
#define EDI (&x86regs[5])
#define ESP (&x86regs[6])
#define EBP (&x86regs[7])

struct x86reg x86regs[] = {
    {"eax", NULL, 0},
    {"ebx", NULL, 0},
    {"ecx", NULL, 0},
    {"edx", NULL, 0},
    {"esi", NULL, 0},
    {"edi", NULL, 0},
    {"esp", NULL, 0},
    {"ebp", NULL, 0},
};

struct x86_instruction instruction_set[] = {
    {X86_MOV, "mov %s,%s"},
    {X86_ADD, "add %s,%s"},
    {X86_SUB, "sub %s,%s"},
    {X86_MUL, "mul %s"},
    {X86_DIV, "div %s"},
    {X86_NEG, "neg %s"},
    {X86_JMP, "jmp %s"},
    {X86_JG, "jg %s"},
    {X86_JGE, "jge %s"},
    {X86_JL, "jl %s"},
    {X86_JLE, "jle %s"},
    {X86_JE, "je %s"},
    {X86_JNE, "jne %s"},
    {X86_CMP, "cmp %s,%s"},
    {X86_CALL, "call %s"},
    {X86_RET, "ret"},
    {X86_PUSH, "push %s"},
    {X86_POP, "pop %s"},
    {X86_LEAVE, "leave"},
    {X86_LEA, "lea %s,%s"},
    {X86_PUSHAD, "pushad"},
    {X86_POPAD, "popad"},
};

static int localvar_nr = 0;

void initial_address(struct address * addr, int type, struct x86reg * reg,
                     struct x86reg * base, int offset)
{
    addr->type = type;
    addr->reg = reg;
    addr->base = base;
    addr->offset = offset;
    addr->dirty = 0;
}

static char * address_tostring(struct address * addr, int type, char * dest)
{
    switch(type) {
    case ADDR_SYMBOL:
        sprintf(dest, "DWORD [%s]", addr->sym->name);
        return dest;
    case ADDR_SYMBOL_NO_DWORD:
        sprintf(dest, "[%s]", addr->sym->name);
        return dest;
    case ADDR_REG:
        sprintf(dest, "%s", addr->reg->name);
        return dest;
    case ADDR_REG_RELATIVE:
        sprintf(dest, "DWORD [%s%+d]", addr->base->name, addr->offset);
        return dest;
    case ADDR_REG_RELATIVE_NO_DWORD:
        sprintf(dest, "[%s%+d]", addr->base->name, addr->offset);
        return dest;
    case ADDR_IMM:
        sprintf(dest, "%d", addr->offset);
        return dest;
    default:
        return NULL;
    }
}

static void print_variable(FILE * output, char * symbol)
{
    fprintf(output, "%s: DD 0\n", symbol);
}

static void print_label(FILE * output, char * label)
{
    fprintf(output, "%s:\n", label);
}

static void print_instruction(FILE * output, int opcode, ...)
{
    char format[128];
    sprintf(format, "\t%s\n", instruction_set[opcode].format);
    va_list ap;
    va_start(ap, opcode);
    vfprintf(output, format, ap);
    va_end(ap);
}

static int generate_esp_op(int op, int n)
{
    struct address esp, imm;
    char espstr[8], immstr[16];
    esp.reg = ESP;
    imm.offset = n;
    address_tostring(&esp, ADDR_REG, espstr);
    address_tostring(&imm, ADDR_IMM, immstr);
    print_instruction(x86output, op, espstr, immstr);
    return 0;
}

static int generate_twoop_instruction(int opcode, struct address * dest_addr,
                                      int dest_type, struct address * src_addr, int src_type)
{
    char dest_str[ID_MAX_LEN], src_str[ID_MAX_LEN];
    address_tostring(dest_addr, dest_type, dest_str);
    address_tostring(src_addr, src_type, src_str);
    print_instruction(x86output, opcode, dest_str, src_str);
    return 0;
}
//该函数生成为变量分配空间，生成过程入口指令
static int generate_proc_begin(struct statements_set * ss)
{
    struct mid_statement * ms;
    int ins_begin = 0;
    char * proc_label;

    if(global_scope->ss == ss) {
        proc_label = MAIN;
        //全局变量最先申明
        while(1) {
            ms = &ss->statements[ins_begin];
            if(ms->type != MIDOP_ALLOC)
                break;
            print_variable(x86output, ms->dest->name);
            initial_address(&ms->dest->addr, ADDR_SYMBOL, NULL, NULL, 0);
            ins_begin++;
        }
        fprintf(x86output, "section .code\n");
    } else {
        //过程的第一条语句应是CREAT PROC
        proc_label = ss->statements[0].dest->name;
    }
    print_label(x86output, proc_label);

    struct address ebp, esp;
    char ebpstr[8], espstr[8];
    ebp.reg = EBP;
    esp.reg = ESP;
    address_tostring(&ebp, ADDR_REG, ebpstr);
    address_tostring(&esp, ADDR_REG, espstr);
    print_instruction(x86output, X86_PUSH, ebpstr);
    print_instruction(x86output, X86_MOV, ebpstr, espstr);

    //统计过程局部变量数，在栈分配空间
    if(global_scope->ss != ss) {
        ins_begin = 1;
        //保存现场
        print_instruction(x86output, X86_PUSHAD);
        //pushad共推进7个寄存器
        localvar_nr += PUSHAD_REGS;
        while(1) {
            ms = &ss->statements[ins_begin];
            if(ms->type != MIDOP_ALLOC)
                break;
            int offset;
            offset = - (localvar_nr + 1) * INT_LENGTH;
            initial_address(&ms->dest->addr, ADDR_REG_RELATIVE, NULL, EBP, offset);
            localvar_nr++;
            ins_begin++;
        }
        generate_esp_op(X86_SUB, (localvar_nr - PUSHAD_REGS) * INT_LENGTH);
    }
    //之后从ins_begin开始执行语句
    return ins_begin;
}

static int generate_proc_end(struct statements_set * ss)
{
    if(ss != global_scope->ss) {
        int offset = (localvar_nr - PUSHAD_REGS) * INT_LENGTH;
        generate_esp_op(X86_ADD, offset);
        print_instruction(x86output, X86_POPAD);
    }
    print_instruction(x86output, X86_LEAVE);
    print_instruction(x86output, X86_RET);
    return 0;
}

static int generate_header()
{
    fprintf(x86output, "extern %s,%s\n", PRINTF, SCANF);
    fprintf(x86output, "global %s\n", MAIN);
    fprintf(x86output, "section .data\n");
    fprintf(x86output, "_number_write_str: DB 0x25,0x64,0x0a,0x0\n");
    fprintf(x86output, "_number_read_str: DB 0x25,0x64,0x0\n");
    return 0;
}

#define USEABLE_REG_NR 6

//获得符号在内存中的寻址方式 有ADDR_SYMBOL和ADDR_REG_RELATIVE两种
static int get_symbol_memory_type(struct symbol * sym)
{
    if(sym->type == SYM_TYPE_VAR)
        return sym->addr.type;
    else if(sym->type == SYM_TYPE_NUMBER)
        return ADDR_IMM;
    else
        return ADDR_REG_RELATIVE;
}

static int get_address_type(struct address * addr)
{
    if(addr->reg)
        return ADDR_REG;
    else
        return get_symbol_memory_type(addr->sym);
}

static void dirty_sym_write_back(struct symbol * sym, struct x86reg * reg)
{
    int type;
    struct address reg_addr;
    type = get_symbol_memory_type(sym);
    initial_address(&reg_addr, ADDR_REG, reg, NULL, 0);
    generate_twoop_instruction(X86_MOV, &sym->addr, type,
                               &reg_addr, ADDR_REG);
    sym->addr.dirty = 0;
}

static struct x86reg * get_reg(struct x86reg * request_reg) {
    struct symbol * sym;
    struct x86reg * reg;
    if(!request_reg) {
        //没有使用的寄存器可直接使用
        for(int i = 0; i < USEABLE_REG_NR; i++)
            if(!x86regs[i].lock && x86regs[i].sym == NULL)
                return &x86regs[i];

        //优先使用已分配过内存并且不脏的变量
        for(int i = 0; i < USEABLE_REG_NR; i++) {
            reg = &x86regs[i];
            sym = reg->sym;
            if(!reg->lock && sym->addr.base != NULL)
                if(sym->addr.dirty == 0) {
                    sym->addr.reg = NULL;
                    reg->sym = NULL;
                    return reg;
                }
        }
        for(int i = 0; i < USEABLE_REG_NR; i++) {
            reg = &x86regs[i];
            sym = reg->sym;
            if(!reg->lock && sym->addr.base != NULL) {
                dirty_sym_write_back(sym, reg);
                sym->addr.reg = NULL;
                reg->sym = NULL;
                return reg;
            }
        }
        //所有寄存器都分配了，指定换出临时变量
        for(int i = 1; i < USEABLE_REG_NR; i++)
            if(!x86regs[i].lock)
                request_reg = &x86regs[i];
    }

    //分配指定的寄存器
    if(request_reg && !request_reg->lock) {
        sym = request_reg->sym;
        if(sym == NULL)
            return request_reg;

        if(sym->addr.base && sym->addr.dirty == 0) {
            //不脏直接返回
            sym->addr.reg = NULL;
            request_reg->sym = NULL;
            return request_reg;
        }

        //脏需要写回
        //没有分配内存？先分配
        if(sym->addr.base == NULL) {
            localvar_nr++;
            sym->addr.base = EBP;
            sym->addr.offset = - localvar_nr * INT_LENGTH;
            generate_esp_op(X86_SUB, 4);
        }
        dirty_sym_write_back(sym, request_reg);

        sym->addr.reg = NULL;
        request_reg->sym = NULL;
        return request_reg;
    }
    return NULL;
}

static int compile_mov_add_sub_model(struct mid_statement * ms, int opcode)
{
    struct address * dest_addr, * src_addr;
    dest_addr = &ms->dest->addr;
    src_addr = &ms->src->addr;

    //目标在寄存器中
    if(dest_addr->reg || src_addr->type == ADDR_IMM) {
        int src_type;
        src_type = get_address_type(src_addr);
        generate_twoop_instruction(opcode, dest_addr, ADDR_REG, src_addr, src_type);
        dest_addr->dirty = 1;
    } else {
        int dest_type;
        dest_type = get_symbol_memory_type(ms->dest);
        //目标和源操作数都不在寄存器中，将源调入寄存器
        if(!src_addr->reg) {
            struct x86reg * reg;
            struct address reg_address;
            int src_type;

            src_type = get_symbol_memory_type(src_addr->sym);
            reg = get_reg(NULL);
            initial_address(&reg_address, ADDR_REG, reg, NULL, 0);
            generate_twoop_instruction(X86_MOV, &reg_address, ADDR_REG, src_addr, src_type);

            src_addr->reg = reg;
            reg->sym = src_addr->sym;

        }
        generate_twoop_instruction(opcode, dest_addr, dest_type, src_addr, ADDR_REG);
    }
    return 0;
}

static int compile_mul_div_model(struct mid_statement * ms, int opcode)
{
    struct address * dest_addr, * src_addr;
    dest_addr = &ms->dest->addr;
    src_addr = &ms->src->addr;
    struct x86reg * eax, * edx;
    struct address addr;

    int mod = 0;
    if(opcode == X86_MOD) {
        mod = 1;
        opcode = X86_DIV;
    }

    //乘除法目标操作数必须是eax
    if(dest_addr->reg != EAX) {
        int dest_type;
        dest_type = get_address_type(dest_addr);
        eax = get_reg(EAX);
        initial_address(&addr, ADDR_REG, eax, NULL, 0);
        generate_twoop_instruction(X86_MOV, &addr, ADDR_REG, dest_addr, dest_type);

        if(dest_addr->reg)
            dest_addr->reg->sym = NULL;
        dest_addr->reg = eax;
        eax->sym = dest_addr->sym;
    }

    EAX->lock = 1;
    //除法时EDX必须为0
    edx = get_reg(EDX);
    EDX->lock = 1;
    initial_address(&addr, ADDR_REG, edx, NULL, 0);
    struct address imm;
    initial_address(&imm, ADDR_IMM, NULL, NULL, 0);
    generate_twoop_instruction(X86_MOV, &addr, ADDR_REG, &imm, ADDR_IMM);

    //源操作数不能是立即数
    struct x86reg * reg = NULL;
    if(src_addr->type == ADDR_IMM) {
        reg = get_reg(NULL);
        initial_address(&addr, ADDR_REG, reg, NULL, 0);
        generate_twoop_instruction(X86_MOV, &addr, ADDR_REG, src_addr, ADDR_IMM);
        reg->sym = src_addr->sym;
        src_addr->reg = reg;
    }

    int src_type;
    src_type = get_address_type(src_addr);
    char srcstr[ID_MAX_LEN];
    address_tostring(src_addr, src_type, srcstr);
    print_instruction(x86output, opcode, srcstr);

    //需要取余数？
    if(mod) {
        initial_address(&addr, ADDR_REG, edx, NULL, 0);
        generate_twoop_instruction(X86_MOV, dest_addr, ADDR_REG, &addr, ADDR_REG);
    }
    EAX->lock = 0;
    EDX->lock = 0;
    edx->sym = NULL;
    if(reg)
        reg->sym = NULL;
    dest_addr->dirty = 1;
    return 0;
}

static int compile_midop_assign(struct mid_statement * ms)
{
    return compile_mov_add_sub_model(ms, X86_MOV);
}

static int compile_midop_add(struct mid_statement * ms)
{
    return compile_mov_add_sub_model(ms, X86_ADD);
}

static int compile_midop_sub(struct mid_statement * ms)
{
    return compile_mov_add_sub_model(ms, X86_SUB);
}

static int compile_midop_mul(struct mid_statement * ms)
{
    return compile_mul_div_model(ms, X86_MUL);
}

static int compile_midop_div(struct mid_statement * ms)
{
    return compile_mul_div_model(ms, X86_DIV);
}

static int compile_midop_mod(struct mid_statement * ms)
{
    return compile_mul_div_model(ms, X86_MOD);
}

static int compile_midop_neg(struct mid_statement * ms)
{
    struct address * dest;
    dest = &ms->dest->addr;
    int type;
    char deststr[ID_MAX_LEN];
    type = get_address_type(dest);
    address_tostring(dest, type, deststr);
    print_instruction(x86output, X86_NEG, deststr);
    return 0;
}

static int compile_midop_compare(struct mid_statement * ms)
{
    return compile_mov_add_sub_model(ms, X86_CMP);
}

#define CALL_PRINTF 1
#define CALL_SCANF 2

static int generate_call_printf_scanf(int type)
{
    char * strname, * funcname;
    if(type == CALL_PRINTF) {
        strname = "_number_write_str";
        funcname = PRINTF;
    } else if(type == CALL_SCANF) {
        strname = "_number_read_str";
        funcname = SCANF;
    }

    print_instruction(x86output, X86_PUSH, strname);
    print_instruction(x86output, X86_CALL, funcname);

    struct address esp, num;
    initial_address(&esp, ADDR_REG, ESP, NULL, 0);
    initial_address(&num, ADDR_IMM, NULL, NULL, 8);
    generate_twoop_instruction(X86_ADD, &esp, ADDR_REG, &num, ADDR_IMM);
    return 0;
}

static int compile_midop_write(struct mid_statement * ms)
{
    int type;
    char addrstr[ID_MAX_LEN];
    type = get_address_type(&ms->dest->addr);
    address_tostring(&ms->dest->addr, type, addrstr);
    print_instruction(x86output, X86_PUSH, addrstr);
    generate_call_printf_scanf(CALL_PRINTF);
    return 0;
}

static int compile_midop_read(struct mid_statement *ms)
{
    int type;
    char regstr[ID_MAX_LEN];
    struct address reg_addr, * dest_addr;
    struct x86reg * reg;
    dest_addr = &ms->dest->addr;
    reg = get_reg(NULL);
    initial_address(&reg_addr, ADDR_REG, reg, NULL, 0);
    type = get_address_type(dest_addr);

    if(type == ADDR_SYMBOL)
        type = ADDR_SYMBOL_NO_DWORD;
    else if(type == ADDR_REG_RELATIVE)
        type = ADDR_REG_RELATIVE_NO_DWORD;

    generate_twoop_instruction(X86_LEA, &reg_addr, ADDR_REG, dest_addr, type);

    address_tostring(&reg_addr, ADDR_REG, regstr);
    print_instruction(x86output, X86_PUSH, regstr);

    generate_call_printf_scanf(CALL_SCANF);
    return 0;
}


static int get_compare_opcode(char * compare_symbol)
{
    if(strcmp(compare_symbol, "<") == 0)
        return X86_JGE;
    else if(strcmp(compare_symbol, "<=") == 0)
        return X86_JG;
    else if(strcmp(compare_symbol, "==") == 0)
        return X86_JNE;
    else if(strcmp(compare_symbol, "#") == 0)
        return X86_JE;
    else if(strcmp(compare_symbol, ">") == 0)
        return X86_JLE;
    else if(strcmp(compare_symbol, ">=") == 0)
        return X86_JL;
}

int compile_statement_set(struct statements_set * ss)
{
    localvar_nr = 0;
    int idx = 0;
    struct mid_statement * ms;
    struct x86reg * reg;
    idx = generate_proc_begin(ss);
    int compare_opcode = 0;

    for(; idx < ss->count; idx++) {
        ms = &ss->statements[idx];
        switch(ms->type) {
        case MIDOP_ASSIGN:
            compile_midop_assign(ms);
            break;
        case MIDOP_ADD:
            compile_midop_add(ms);
            break;
        case MIDOP_MINUS:
            compile_midop_sub(ms);
            break;
        case MIDOP_MULTIPLY:
            compile_midop_mul(ms);
            break;
        case MIDOP_DIVIDE:
            compile_midop_div(ms);
            break;
        case MIDOP_MOD:
            compile_midop_mod(ms);
            break;
        case MIDOP_NEG:
            compile_midop_neg(ms);
            break;
        case MIDOP_COMPARE:
            compile_midop_compare(ms);
            break;
        case MIDOP_CREATE_LABEL:
            print_label(x86output, ms->dest->name);
            break;
        case MIDOP_GOTO:
            print_instruction(x86output, X86_JMP, ms->dest->name);
            break;
        case MIDOP_CALL:
            print_instruction(x86output, X86_CALL, ms->dest->name);
            break;
        case MIDOP_SET_COMPARE_METHOD:
            compare_opcode = get_compare_opcode(ms->dest->name);
            break;
        case MIDOP_IF:
            print_instruction(x86output, compare_opcode, ms->dest->name);
            break;
        case MIDOP_WRITE:
            compile_midop_write(ms);
            break;
        case MIDOP_READ:
            compile_midop_read(ms);
            break;
        case MIDOP_GETTEMP:
            reg = get_reg(NULL);
            ms->dest->addr.reg = reg;
            reg->sym = ms->dest;
            break;
        case MIDOP_DROPTEMP:
            if(ms->dest->addr.reg)
                ms->dest->addr.reg->sym = NULL;
            break;
        }
    }
    generate_proc_end(ss);
    return 0;
}

void compile_scope(struct scope * sc)
{
    if(!sc)
        return;
    compile_statement_set(sc->ss);
    compile_scope(sc->sibling);
    compile_scope(sc->child);
}

int generate_x86asm(char * output)
{
    x86output = fopen(output, "w");
    if(!x86output)
        return 1;

    generate_header();
    compile_scope(global_scope);
    fflush(x86output);
    fclose(x86output);
    return 0;
}