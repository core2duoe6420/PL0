#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"

struct buffer * buffer_create(FILE * file) {
    struct buffer * buf;
    buf = (struct buffer *)malloc(sizeof(struct buffer));
    if(!buf)
        return NULL;
    buf->file = file;
    fseek(file, 0, SEEK_END);
    buf->file_len = ftell(file);
    fseek(file, 0, SEEK_SET);
    buf->offset = 0;
    buf->buf_id = 0;
    buf->next = 0;
    fread(buf->buf[0], sizeof(char), BUFFER_SIZE, file);
    return buf;
}

void buffer_drop(struct buffer ** buf)
{
    fclose((*buf)->file);
    free(*buf);
    *buf = NULL;
}

char buffer_get_next(struct buffer * buf)
{
    if(buf->file_len == buf->offset)
        return EOF;
    if(buf->next == BUFFER_SIZE) {
        buf->next = 0;
        buf->buf_id = 1 - buf->buf_id;
        fread(buf->buf[buf->buf_id], sizeof(char), BUFFER_SIZE, buf->file);
    }

    char ret;
    ret = buf->buf[buf->buf_id][buf->next];
    buf->next++;
    buf->offset++;
    return ret;
}

void buffer_put_back(struct buffer * buf)
{
    if(!buf->offset)
        return;
    if(buf->next == 1 && buf->offset > BUFFER_SIZE - 1) {
        buf->next = BUFFER_SIZE;
        buf->buf_id = 1 - buf->buf_id;
        fseek(buf->file, -BUFFER_SIZE, SEEK_CUR);
    }
    buf->next--;
    buf->offset--;
}

struct word_table * word_table_create() {
    struct word_table * wt;
    wt = (struct word_table *)malloc(sizeof(struct word_table));
    wt->max = 10;
    wt->words = (struct word *)malloc(sizeof(struct word)*wt->max);
    wt->count = 0;
    wt->identifiers.max = ST_INCREMENT;
    wt->identifiers.offset = 0;
    wt->identifiers.buffer = (char *)malloc(sizeof(char)*wt->identifiers.max);
    return wt;
}

int string_table_add(struct string_table * st, char * str)
{
    int len;
    int ret;

    len = strlen(str);
    ret = st->offset;

    while(1 + len + st->offset > st->max) {
        st->buffer = (char *)realloc(st->buffer, sizeof(char) * (st->max + ST_INCREMENT));
        st->max += ST_INCREMENT;
    }
    strcpy(st->buffer + st->offset, str);
    st->offset += len + 1;

    return ret;
}

void word_table_drop(struct word_table ** wt)
{
    free((*wt)->identifiers.buffer);
    free((*wt)->words);
    free(*wt);
    *wt = NULL;
}

int word_table_add(struct word_table * wt, struct word * w, char * id_name)
{
    if(wt->count == wt->max) {
        wt->words = (struct word*)realloc(wt->words, sizeof(struct word) * (wt->max * 2));
        wt->max = wt->max * 2;
    }

    memcpy(wt->words + wt->count, w, sizeof(struct word));
    wt->words[wt->count].table = wt;

    if((w->type->word_type == TYPE_IDENTIFIER
        || w->type->word_type == TYPE_IDOVERLENGTH
        || w->type->word_type == TYPE_UNDEFINED) && id_name) {
        int offset = string_table_add(&wt->identifiers, id_name);
        wt->words[wt->count].lex_offset = offset;
    }

    wt->count++;

    return 0;
}

void word_table_print(struct word_table * wt)
{
    for(int i = 0; i < wt->count; i++) {
        printf("%d\t%d\t%12s\t", i + 1, wt->words[i].row, wt->words[i].type->print_name);
        if(wt->words[i].type->word_type == TYPE_DOUBLE)
            ;//printf("%lf\t", wt->words[i].dvalue);
        else if(wt->words[i].type->word_type == TYPE_INTEGER)
            printf("%d\t", wt->words[i].ivalue);
        else if(wt->words[i].type->word_type == TYPE_IDENTIFIER
                || wt->words[i].type->word_type == TYPE_UNDEFINED
                || wt->words[i].type->word_type == TYPE_IDOVERLENGTH)
            printf("%s\t", identname(&wt->words[i]));
        printf("\n");
    }
    printf("\n");
}