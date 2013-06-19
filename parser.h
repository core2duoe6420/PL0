#ifndef __PARSE_H
#define __PARSE_H

#define VN_PROGRAM 'A'
#define VN_DIV_PROGRAM 'B'
#define VN_CONST_CLAIM 'C'
#define VN_VAR_CLAIM 'D'
#define VN_PROC_CLAIM 'E'
#define VN_STATEMENT 'F'
#define VN_CONST_PART 'G'
#define VN_VAR_PART 'H'
#define VN_EXPRESSION 'I'
#define VN_STAT_PART_ONE 'J'
#define VN_STAT_PART_TWO 'K'
#define VN_STAT_PART_THR 'L'
#define VN_CONDITION 'M'
#define VN_EXP_PART_ONE 'N'
#define VN_EXP_PART_TWO 'O'
#define VN_ITEM 'P'
#define VN_FACTOR 'Q'
#define VN_ITEM_PART 'R'

#define VN_START_SYM VN_PROGRAM
#define VN_END_SYM VN_ITEM_PART

#define VT_PROC_END 'a'
#define VT_CONST 'b'
#define VT_IDENTIFIER 'c'
#define VT_NUM 'd'
#define VT_COMMA 'e'
#define VT_CONSTASSIGN 'f'
#define VT_ASSIGN 'g'
#define VT_SEMICOLON 'h'
#define VT_VAR 'i'
#define VT_PROCEDURE 'j'
#define VT_CALL 'k'
#define VT_WHILE 'l'
#define VT_READ 'm'
#define VT_WRITE 'n'
#define VT_LBRACKETS 'o'
#define VT_RBRACKETS 'p'
#define VT_BEGIN 'q'
#define VT_END 'r'
#define VT_IF 's'
#define VT_ELSE 't'
#define VT_ODD 'u'
#define VT_COMPARE 'v'
#define VT_PLUS_MINUS 'w'
#define VT_MUL_DIV 'x'
#define VT_THEN 'y'
#define VT_DO 'z'
#define VT_EMPTY '$'
#define VT_STAB '#'

#define VT_START_SYM VT_PROC_END
#define VT_END_SYM VT_DO


#define VN_NR 18
#define VT_NR 26

extern int analyze_parse(struct word_table * wt);
extern int analyze_parse_rec_sub(struct word_table * wt);

#endif