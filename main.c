#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "sym_table.h"
#include "midcode.h"

#ifndef __LINUX__
#define OBJ_SUFFIX ".obj"
#else
#define OBJ_SUFFIX ".o"
#endif

extern int compile_asm_link(char * asm_file, char * obj_file, char * exe_file);

void fill_suffix(char * code_file, char * str, char * suffix)
{
    int len = strlen(code_file);
    int point = 0;
    for(point = len - 1; point >= 0; point--)
        if(code_file[point] == '.')
            break;

    if(point < 0)
        point = len;

    for(int i = 0; i < point; i++)
        str[i] = code_file[i];

    strcpy(str + point, suffix);
}

#define WORD_TABLE_FILE 1
#define SYMBOL_TABLE_FILE 2
#define MIDDLE_CODE_FILE 3

void redirect_output_file(int type, char * output_file, struct word_table * wt)
{
    if(output_file) {
        if(freopen(output_file, "w", stdout) == NULL) {
            printf("can't open file %s\n", output_file);
            exit(1);
        }
        switch(type) {
        case WORD_TABLE_FILE:
            word_table_print(wt);
            break;
        case SYMBOL_TABLE_FILE:
            scope_print_tree(global_scope);
            break;
        case MIDDLE_CODE_FILE:
            statements_set_print_all_scope(global_scope);
            break;
        }

        char * console;
#ifndef __LINUX__
        console = "CON";
#else
        console = "/dev/console";
#endif
        if(freopen(console, "w", stdout) == NULL) {
            exit(1);
        }
    }
}

void test_input_file(char * argv, char * arg)
{
    if(argv == NULL) {
        printf("%s: no input file\n", arg);
        exit(1);
    }
}

int main(int argc, char * argv[])
{
    FILE * file;
    char * code_file = NULL, * symtable_file = NULL;
    char * midcode_file = NULL, asm_file[128] = {0};
    char * wordtable_file = NULL, * exe_file = NULL;
    char obj_file[128] = {0};
    for(int i = 1; i < argc; i++) {
        if(argv[i][0] == '-') {
            if(strcmp(argv[i], "-t") == 0) {
                test_input_file(argv[i + 1], argv[i]);
                symtable_file = argv[++i];
            } else if(strcmp(argv[i], "-m") == 0) {
                test_input_file(argv[i + 1], argv[i]);
                midcode_file = argv[++i];
            } else if(strcmp(argv[i], "-s") == 0) {
                test_input_file(argv[i + 1], argv[i]);
                strcpy(asm_file, argv[++i]);
            } else if(strcmp(argv[i], "-w") == 0) {
                test_input_file(argv[i + 1], argv[i]);
                wordtable_file = argv[++i];
            } else if(strcmp(argv[i], "-o") == 0) {
                test_input_file(argv[i + 1], argv[i]);
                exe_file = argv[++i];
            } else if(strcmp(argv[i], "-h") == 0) {
                printf("PL0 compiler help:\n"
                       "-t: print symbol table to argument file\n"
                       "-w: print lexical analysis result to argument file\n"
                       "-m: print middle code to argument file\n"
                       "-s: print x86 assembly code to argument file\n"
                       "-o: set execute file name\n");
                exit(0);
            } else {
                printf("%s: unknown argument\n", argv[i]);
                exit(1);
            }
        } else {
            code_file = argv[i];
        }
    }
    if(!code_file) {
        printf("PL0: no input file\n");
        exit(1);
    }
    file = fopen(code_file, "rb");
    if(!file) {
        printf("can't open file %s\n", code_file);
        exit(1);
    }

    int del_asm = 0;
    if(asm_file[0] == '\0') {
        fill_suffix(code_file, asm_file, ".asm");
        del_asm = 1;
    }
    fill_suffix(code_file, obj_file, OBJ_SUFFIX);

    struct buffer * buf;
    if((buf = buffer_create(file)) == NULL) {
        printf("can't create buffer\n");
        exit(1);
    }
    struct word_table * wt;
    wt = analyze_lexical(buf);

    if(analyze_parse_rec_sub(wt)) {
        printf("PL0: parsing fail\n");
        exit(1);
    }

    redirect_output_file(WORD_TABLE_FILE, wordtable_file, wt);
    redirect_output_file(SYMBOL_TABLE_FILE, symtable_file, NULL);
    redirect_output_file(MIDDLE_CODE_FILE, midcode_file, NULL);

    if(generate_x86asm(asm_file)) {
        printf("can't open file %s\n", asm_file);
        exit(1);
    }

    compile_asm_link(asm_file, obj_file, exe_file);

    if(del_asm)
        remove(asm_file);

    word_table_drop(&wt);
    buffer_drop(&buf);
    light_rbtree_delete(terminals_rbtree);
    scope_delete_tree(global_scope);
    scope_delete_tree(temp_scope);
    return 0;
}
