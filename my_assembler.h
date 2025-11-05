#ifndef MY_ASSEMBLER_H
#define MY_ASSEMBLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINES 5000 
#define MAX_INST 256
#define MAX_OPERAND 3
#define MAX_LINE_LENGTH 256

char *input_data[MAX_LINES];
static int line_num = 0; 

struct token_unit {
    char *label;
    char *operator;
    char operand[MAX_OPERAND][20];
    char comment[100];
    int operand_count;
};

typedef struct token_unit token ; 
token *token_table[MAX_LINES] ; 

struct inst_unit{
    char str[10];
    unsigned char op;
    int format;
    int ops;
};

typedef struct inst_unit inst;
inst *inst_table[MAX_INST];
int inst_index = 0; 

void load_inst_table(const char *filename);
void load_source_code(const char *filename);
void parse_source_code();
inst *find_opcode(const char *operator_str);
void print_parsed_output();
void free_all_memory();
char *my_strdup(const char *s);

#endif
