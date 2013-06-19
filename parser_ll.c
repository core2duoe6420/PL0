/* PL0�﷨���� LL(1)�ķ� Ԥ�������
 * ���� => �ֳ��� .
 * �ֳ��� => �������� �������� �������� ���
 * �������� => ��
 * �������� => const ident = num ����part ;
 * ����part => ��
 * ����part => , ident = num ����part
 * �������� => ��
 * �������� => var ident ����part ;
 * ����part => ��
 * ����part => , ident ����part
 * �������� => ��
 * �������� => procedure ident ; �ֳ��� ; ��������
 * ��� => ident := ���ʽ ;
 * ��� => call ident
 * ��� => { ��� ���part1 }
 * ���part1 => ��
 * ���part1 => ; ��� ���part1
 * ��� => while ( ���� ) ���
 * ��� => read ( ident ����part )
 * ��� => write ( ���ʽ ���part2 )
 * ���part2 => ��
 * ���part2 => , ���ʽ
 * ��� => if ( ���� ) ��� ���part3
 * ���part3 => ��
 * ���part3 => else ���
 * ���� => odd ���ʽ
 * ���� => ���ʽ �� ���ʽ
 * ���ʽ => �� ���ʽpart1
 * ���ʽ => ���ʽpart1
 * ���ʽpart1 => �� ���ʽpart2
 * ���ʽpart2 => ��
 * ���ʽpart2 => �� �� ���ʽpart2
 * �� => ���� ��part
 * ��part => ��
 * ��part => �� ���� ��part
 * ���� => ident
 * ���� => num
 * ���� => ( ���ʽ )

 * �գ��Ƚ������ == # < <= > >=
 * �̣��Ӽ��� + -
 * �أ��˳��� * /

 * ���ս����
 * A����
 * B�ֳ���
 * C��������
 * D��������
 * E��������
 * F���
 * G����part
 * H����part
 * I���ʽ
 * J���part1
 * K���part2
 * L���part3
 * M����
 * N���ʽpart1
 * O���ʽpart2
 * P��
 * Q����
 * R��part

 * �ս��
 * a .
 * b const
 * c ident
 * d num
 * e ,
 * f =
 * g :=
 * h ;
 * i var
 * j procedure
 * k call
 * l while
 * m read
 * n write
 * o (
 * p )
 * q {
 * r }
 * s if
 * t else
 * u odd
 * v ��
 * w ��
 * x ��

 * ת���������ʽ
 * A=>Ba
 * B=>CDEF
 * C=>z
 * C=>bcfdGh
 * G=>z
 * G=>ecfdG
 * D=>z
 * D=>icHh
 * H=>z
 * H=>ecH
 * E=>z
 * E=>jchBhE
 * F=>cgIh
 * F=>kc
 * F=>qFJr
 * J=>z
 * J=>hFJ
 * F=>loMpF
 * F=>mocHp
 * F=>noIKp
 * K=>z
 * K=>eI
 * F=>soMpFL
 * L=>z
 * L=>tF
 * M=>uI
 * M=>IvI
 * I=>wN
 * I=>N
 * N=>PO
 * O=>z
 * O=>wPO
 * P=>QR
 * R=>z
 * R=>xQR
 * Q=>c
 * Q=>d
 * Q=>oIp

 * FIRST(A)	b i j c k q l m n s
 * FIRST(B)	b i j c k q l m n s
 * FIRST(C)	�� b
 * FIRST(D)	�� i
 * FIRST(E)	�� j
 * FIRST(F)	c k q l m n s
 * FIRST(G)	�� e
 * FIRST(H)	�� e
 * FIRST(I)	w c d o
 * FIRST(J)	�� h
 * FIRST(K)	�� e
 * FIRST(L)	�� t
 * FIRST(M)	u w c d o
 * FIRST(N)	c d o
 * FIRST(O)	�� w
 * FIRST(P)	c d o
 * FIRST(Q)	c d o
 * FIRST(R)	�� x

 * FOLLOW(A)	#
 * FOLLOW(B)	a h
 * FOLLOW(C)	i j c k q l m n s
 * FOLLOW(D)	j c k q l m n s
 * FOLLOW(E)	c k q l m n s
 * FOLLOW(F)	h t r a
 * FOLLOW(G)	h
 * FOLLOW(H)	h p
 * FOLLOW(I)	h e p v
 * FOLLOW(J)	r
 * FOLLOW(K)	p
 * FOLLOW(L)	h t r a
 * FOLLOW(M)	p
 * FOLLOW(N)	h e p v
 * FOLLOW(O)	h e p v
 * FOLLOW(P)	w h e p v
 * FOLLOW(Q)	x w h e p v
 * FOLLOW(R)	w h e p v
 */

/* �޸ĵĵط���
 * 1.begin��end��Ϊ { } -�����ɴʷ��������
 * 2.��ֵ����Ҫ���һ���ֺ�
 * 3.����else���
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "light_stack.h"
#include "lexer.h"
#include "parser.h"

extern char * type_name[];

#define get_prod(vn,vt) (analysis_forecast_table[(vn)-'A'][(vt-'a')])

#define is_vt(v) ((v>=VT_START_SYM && v<=VT_END_SYM) || v==VT_EMPTY || v==VT_STAB)
#define is_vn(v) (v>=VN_START_SYM && v<=VN_END_SYM)

static char * vn_try_fix[] = {
    /* A-E */	NULL, NULL, NULL, NULL, NULL,
    /* F-J */	NULL, ",", ";", NULL, NULL,
    /* K-O */	",", "else", NULL, NULL, NULL,
    /* P-R */	NULL, NULL, NULL,
};

static char * err_vn[] = {
    /* A���� */			"\"const, var, procedure, identifier, call, while, read, write, if, '{'\"",
    /* B�ֳ��� */		"\"const, var, procedure, identifier, call, while, read, write, if, '{'\"",
    /* C�������� */		"\"const\"",
    /* D�������� */		"\"var\"",
    /* E�������� */		"\"procedure\"",
    /* F��� */			"\"identifier, call, while, read, write, if, '{'\"",
    /* G����part */		"','",
    /* H����part */		"','",
    /* I���ʽ */		"\"identifier, number, '(', '+', '-'\"",
    /* J���part1 */		"';'",
    /* K���part2 */		"','",
    /* L���part3 */		"\"else\"",
    /* M���� */			"\"odd, identifier, number, '(', '+', '-'\"",
    /* N���ʽpart1*/	"\"identifier, number, '('\"",
    /* O���ʽpart2 */	"\"'+', '-'\"",
    /* P�� */			"\"identifier, number, '('\"",
    /* Q���� */			"\"identifier, number, '('\"",
    /* R��part */		"'*', '/'",
};

static char * err_vt[] = {
    /* 0-3 */	"'.'", "const", "identifier", "number",
    /* 4-7 */	"','", "'='", "':='", "';'",
    /* 8-11 */	"var", "procedure", "call", "while",
    /* 12-15 */	"read", "write", "'('", "')'",
    /* 16-19 */	"'{'", "'}'", "if", "else",
    /* 20-23 */	"odd", "'==' '#' '<' '<=' '>' '>='", "'+' '-'", "'*' '/'",
};

struct production {
    char vn;
    char formula[10];
};

struct production_table {
    int count;
    int max;
    struct production * prod;
};

static struct production * analysis_forecast_table[VN_NR][VT_NR + 1] = {{NULL,}};

static struct production_table * production_table_create(int count) {
    struct production_table * pt;
    pt = (struct production_table *)malloc(sizeof(struct production_table));
    pt->max = count;
    pt->count = 0;
    pt->prod = (struct production *)malloc(sizeof(struct production)*count);
    return pt;
}

static void production_table_delete(struct production_table * pt)
{
    free(pt->prod);
    free(pt);
}

static void production_table_add(struct production_table * pt, char * prod)
{
    char vn = prod[0];
    //����ʽ�ĽṹΪA=>X
    char * form = prod + 3;
    pt->prod[pt->count].vn = vn;
    strcpy(pt->prod[pt->count].formula, form);
    pt->count++;
}

static struct production * production_table_get_prod(struct production_table * pt, char * prod) {
    char vn = prod[0];

#ifdef DEBUG
    if(vn < 'A' || vn > 'Z')
        printf("%s vn not capital\n", prod);
#endif

    char * form = prod + 3;
    for(int i = 0; i < pt->count; i++) {
        if(pt->prod[i].vn == vn && strcmp(pt->prod[i].formula, form) == 0)
            return pt->prod + i;
    }
#ifdef DEBUG
    printf("%s not found\n", prod);
#endif
    return NULL;
}

static struct production_table * pl0_get_production_table() {
    struct production_table * pt;
    pt = production_table_create(38);
    production_table_add(pt, "A=>Ba");
    production_table_add(pt, "B=>CDEF");
    production_table_add(pt, "C=>z");
    production_table_add(pt, "C=>bcfdGh");
    production_table_add(pt, "G=>z");
    production_table_add(pt, "G=>ecfdG");
    production_table_add(pt, "D=>z");
    production_table_add(pt, "D=>icHh");
    production_table_add(pt, "H=>z");
    production_table_add(pt, "H=>ecH");
    production_table_add(pt, "E=>z");
    production_table_add(pt, "E=>jchBhE");
    production_table_add(pt, "F=>cgIh");
    production_table_add(pt, "F=>kc");
    production_table_add(pt, "F=>qFJr");
    production_table_add(pt, "J=>z");
    production_table_add(pt, "J=>hFJ");
    production_table_add(pt, "F=>loMpF");
    production_table_add(pt, "F=>mocHp");
    production_table_add(pt, "F=>noIKp");
    production_table_add(pt, "K=>z");
    production_table_add(pt, "K=>eI");
    production_table_add(pt, "F=>soMpFL");
    production_table_add(pt, "L=>z");
    production_table_add(pt, "L=>tF");
    production_table_add(pt, "M=>uI");
    production_table_add(pt, "M=>IvI");
    production_table_add(pt, "I=>wN");
    production_table_add(pt, "I=>N");
    production_table_add(pt, "N=>PO");
    production_table_add(pt, "O=>z");
    production_table_add(pt, "O=>wPO");
    production_table_add(pt, "R=>z");
    production_table_add(pt, "R=>xQR");
    production_table_add(pt, "Q=>c");
    production_table_add(pt, "Q=>d");
    production_table_add(pt, "Q=>oIp");
    production_table_add(pt, "P=>QR");
    return pt;
}

static void initial_ana_fore_table(struct production_table * pt)
{
    get_prod('A', 'b') = production_table_get_prod(pt, "A=>Ba");
    get_prod('A', 'c') = production_table_get_prod(pt, "A=>Ba");
    get_prod('A', 'i') = production_table_get_prod(pt, "A=>Ba");
    get_prod('A', 'j') = production_table_get_prod(pt, "A=>Ba");
    get_prod('A', 'k') = production_table_get_prod(pt, "A=>Ba");
    get_prod('A', 'l') = production_table_get_prod(pt, "A=>Ba");
    get_prod('A', 'm') = production_table_get_prod(pt, "A=>Ba");
    get_prod('A', 'n') = production_table_get_prod(pt, "A=>Ba");
    get_prod('A', 'q') = production_table_get_prod(pt, "A=>Ba");
    get_prod('A', 's') = production_table_get_prod(pt, "A=>Ba");
    get_prod('B', 'b') = production_table_get_prod(pt, "B=>CDEF");
    get_prod('B', 'c') = production_table_get_prod(pt, "B=>CDEF");
    get_prod('B', 'i') = production_table_get_prod(pt, "B=>CDEF");
    get_prod('B', 'j') = production_table_get_prod(pt, "B=>CDEF");
    get_prod('B', 'k') = production_table_get_prod(pt, "B=>CDEF");
    get_prod('B', 'l') = production_table_get_prod(pt, "B=>CDEF");
    get_prod('B', 'm') = production_table_get_prod(pt, "B=>CDEF");
    get_prod('B', 'n') = production_table_get_prod(pt, "B=>CDEF");
    get_prod('B', 'q') = production_table_get_prod(pt, "B=>CDEF");
    get_prod('B', 's') = production_table_get_prod(pt, "B=>CDEF");
    get_prod('C', 'b') = production_table_get_prod(pt, "C=>bcfdGh");
    get_prod('C', 'd') = production_table_get_prod(pt, "C=>z");
    get_prod('C', 'i') = production_table_get_prod(pt, "C=>z");
    get_prod('C', 'j') = production_table_get_prod(pt, "C=>z");
    get_prod('C', 'k') = production_table_get_prod(pt, "C=>z");
    get_prod('C', 'l') = production_table_get_prod(pt, "C=>z");
    get_prod('C', 'm') = production_table_get_prod(pt, "C=>z");
    get_prod('C', 'n') = production_table_get_prod(pt, "C=>z");
    get_prod('C', 'q') = production_table_get_prod(pt, "C=>z");
    get_prod('C', 's') = production_table_get_prod(pt, "C=>z");
    get_prod('D', 'c') = production_table_get_prod(pt, "D=>z");
    get_prod('D', 'i') = production_table_get_prod(pt, "D=>icHh");
    get_prod('D', 'j') = production_table_get_prod(pt, "D=>z");
    get_prod('D', 'k') = production_table_get_prod(pt, "D=>z");
    get_prod('D', 'l') = production_table_get_prod(pt, "D=>z");
    get_prod('D', 'm') = production_table_get_prod(pt, "D=>z");
    get_prod('D', 'n') = production_table_get_prod(pt, "D=>z");
    get_prod('D', 'q') = production_table_get_prod(pt, "D=>z");
    get_prod('D', 's') = production_table_get_prod(pt, "D=>z");
    get_prod('E', 'c') = production_table_get_prod(pt, "E=>z");
    get_prod('E', 'j') = production_table_get_prod(pt, "E=>jchBhE");
    get_prod('E', 'k') = production_table_get_prod(pt, "E=>z");
    get_prod('E', 'l') = production_table_get_prod(pt, "E=>z");
    get_prod('E', 'm') = production_table_get_prod(pt, "E=>z");
    get_prod('E', 'n') = production_table_get_prod(pt, "E=>z");
    get_prod('E', 'q') = production_table_get_prod(pt, "E=>z");
    get_prod('E', 's') = production_table_get_prod(pt, "E=>z");
    get_prod('F', 'c') = production_table_get_prod(pt, "F=>cgIh");
    get_prod('F', 'k') = production_table_get_prod(pt, "F=>kc");
    get_prod('F', 'l') = production_table_get_prod(pt, "F=>loMpF");
    get_prod('F', 'm') = production_table_get_prod(pt, "F=>mocHp");
    get_prod('F', 'n') = production_table_get_prod(pt, "F=>noIKp");
    get_prod('F', 'q') = production_table_get_prod(pt, "F=>qFJr");
    get_prod('F', 's') = production_table_get_prod(pt, "F=>soMpFL");
    get_prod('G', 'e') = production_table_get_prod(pt, "G=>ecfdG");
    get_prod('G', 'h') = production_table_get_prod(pt, "G=>z");
    get_prod('H', 'e') = production_table_get_prod(pt, "H=>ecH");
    get_prod('H', 'h') = production_table_get_prod(pt, "H=>z");
    get_prod('H', 'p') = production_table_get_prod(pt, "H=>z");
    get_prod('I', 'c') = production_table_get_prod(pt, "I=>N");
    get_prod('I', 'd') = production_table_get_prod(pt, "I=>N");
    get_prod('I', 'o') = production_table_get_prod(pt, "I=>N");
    get_prod('I', 'w') = production_table_get_prod(pt, "I=>wN");
    get_prod('J', 'h') = production_table_get_prod(pt, "J=>hFJ");
    get_prod('J', 'r') = production_table_get_prod(pt, "J=>z");
    get_prod('K', 'e') = production_table_get_prod(pt, "K=>eI");
    get_prod('K', 'p') = production_table_get_prod(pt, "K=>z");
    get_prod('L', 'a') = production_table_get_prod(pt, "L=>z");
    get_prod('L', 'h') = production_table_get_prod(pt, "L=>z");
    get_prod('L', 'r') = production_table_get_prod(pt, "L=>z");
    get_prod('L', 't') = production_table_get_prod(pt, "L=>tF");
    get_prod('M', 'c') = production_table_get_prod(pt, "M=>IvI");
    get_prod('M', 'd') = production_table_get_prod(pt, "M=>IvI");
    get_prod('M', 'o') = production_table_get_prod(pt, "M=>IvI");
    get_prod('M', 'u') = production_table_get_prod(pt, "M=>uI");
    get_prod('M', 'w') = production_table_get_prod(pt, "M=>IvI");
    get_prod('N', 'c') = production_table_get_prod(pt, "N=>PO");
    get_prod('N', 'd') = production_table_get_prod(pt, "N=>PO");
    get_prod('N', 'o') = production_table_get_prod(pt, "N=>PO");
    get_prod('O', 'e') = production_table_get_prod(pt, "O=>z");
    get_prod('O', 'h') = production_table_get_prod(pt, "O=>z");
    get_prod('O', 'p') = production_table_get_prod(pt, "O=>z");
    get_prod('O', 'v') = production_table_get_prod(pt, "O=>z");
    get_prod('O', 'w') = production_table_get_prod(pt, "O=>wPO");
    get_prod('P', 'c') = production_table_get_prod(pt, "P=>QR");
    get_prod('P', 'd') = production_table_get_prod(pt, "P=>QR");
    get_prod('P', 'o') = production_table_get_prod(pt, "P=>QR");
    get_prod('Q', 'c') = production_table_get_prod(pt, "Q=>c");
    get_prod('Q', 'd') = production_table_get_prod(pt, "Q=>d");
    get_prod('Q', 'o') = production_table_get_prod(pt, "Q=>oIp");
    get_prod('R', 'e') = production_table_get_prod(pt, "R=>z");
    get_prod('R', 'h') = production_table_get_prod(pt, "R=>z");
    get_prod('R', 'p') = production_table_get_prod(pt, "R=>z");
    get_prod('R', 'v') = production_table_get_prod(pt, "R=>z");
    get_prod('R', 'w') = production_table_get_prod(pt, "R=>z");
    get_prod('R', 'x') = production_table_get_prod(pt, "R=>xQR");
}

static inline void light_stack_push_production(light_stack stack, struct production * prod)
{
    for(int i = strlen(prod->formula) - 1; i >= 0; i--)
        light_stack_push(stack, &prod->formula[i]);
}

int analyze_parse(struct word_table * wt)
{
    int err = 0;
    struct word * w = wt->words, * w_tmp;
    struct production_table * prod_table;
    int try_fix = 0;
    struct word w_try_fix;

    prod_table = pl0_get_production_table();
    initial_ana_fore_table(prod_table);

    light_stack stack;
    stack = light_stack_create(STACK_INSTANCE, STACK_CAPACITY_AUTO, sizeof(char), 0);

    char v;
    //ѹ��׮����ʼ��
    v = VT_STAB;
    light_stack_push(stack, &v);
    v = VN_PROGRAM;
    light_stack_push(stack, &v);

    while(!light_stack_isempty(stack)) {
        int wrong = 1;
        light_stack_pop(stack, &v);
        if(is_vn(v)) {
            struct production * prod;
            prod = get_prod(v, w->type->vt);
            if(prod) {
                //printf("More prodution: %c=>%s\n", prod->vn, prod->formula);
                light_stack_push_production(stack, prod);
                wrong = 0;
            }
        } else if(is_vt(v)) {
            if(v == VT_EMPTY) {
                //printf("Meet ��\n");
                continue;
            }
            if(v == w->type->vt) {
                //printf("Meet vt: %s\n", w->type->print_name);
                //�Ƿ����޸�״̬��
                if(try_fix) {
                    try_fix = 0;
                    w = w_tmp;
                } else {
                    w++;
                }
                wrong = 0;
#ifdef DEBUG
                if(idx < wt->count) {
                    printf("current word:%s ", type_name[wt->words[idx].type]);
                    if(wt->words[idx].type == TYPE_IDENTIFIER)
                        printf("%s", identname(&wt->words[idx]));
                    printf("\n");
                }
#endif
            }
        }
        if(wrong) {
            err = 1;
            if(v == VT_STAB)
                continue;
            if(is_vn(v)) {
                printf("Error Row %d : expect %s but meet \"%s\"\n",
                       w->row, err_vn[v - VN_START_SYM], word_to_string(w));
                //���ս����Ψһ����ʱ�����޸�
                if(vn_try_fix[v - VN_START_SYM]) {
                    try_fix = 1;
                    w_try_fix.type = get_terminal(vn_try_fix[v - VN_START_SYM]);
                    w_try_fix.row = w->row;
                    w_tmp = w;
                    w = &w_try_fix;
                    light_stack_push(stack, &v);
                }
            } else if(is_vt(v)) {
                printf("Error Row %d : missing expected %s\n",
                       w->row, err_vt[v - VT_START_SYM]);
            } else {
                printf("Error Unknown Row %d\n");
            }
        }
    }

    light_stack_delete(&stack);
    production_table_delete(prod_table);

    if(err)
        printf("\nParsing fail.\n\n");
    else
        printf("\nParsing successfully.\n\n");

    return err;
}