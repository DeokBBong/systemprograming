#include "my_assembler.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <inst_data_file> <source_code_file>\n", argv[0]);
        return 1;
    }

    const char *inst_file = argv[1];
    const char *source_file = argv[2];

    load_inst_table(inst_file);
    load_source_code(source_file);
    parse_source_code();
    print_parsed_output();
    free_all_memory();

    return 0;
}

void load_inst_table(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error opening inst.data file");
        exit(1);
    }

    char name[10];
    char type_char[5]; 
    int format;
    unsigned int opcode_hex;
    
    while (fscanf(fp, "%s %s %d %x", name, type_char, &format, &opcode_hex) == 4) {
        if (inst_index >= MAX_INST) {
            fprintf(stderr, "Warning: Instruction table is full (max %d).\n", MAX_INST);
            break;
        }
        
        inst_table[inst_index] = (inst *)malloc(sizeof(inst));
        if (inst_table[inst_index] == NULL) {
            perror("Memory allocation failed for inst_table");
            exit(1);
        }
        
        strcpy(inst_table[inst_index]->str, name);       
        inst_table[inst_index]->op = (unsigned char)opcode_hex; 
        inst_table[inst_index]->format = format;         
        
        if (strcmp(type_char, "RR") == 0 || strcmp(type_char, "RN") == 0) {
            inst_table[inst_index]->ops = 2; 
        } else if (format == 3 || format == 4 || format == 1 || strcmp(type_char, "R") == 0 || strcmp(type_char, "N") == 0) {
            inst_table[inst_index]->ops = 1; 
        } else {
             inst_table[inst_index]->ops = 0; 
        }
        
        inst_index++;
    }
    fclose(fp);
    printf("Successfully loaded %d instructions.\n", inst_index);
}

void load_source_code(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error opening source code file");
        exit(1);
    }

    char line_buffer[MAX_LINE_LENGTH];
    while (fgets(line_buffer, sizeof(line_buffer), fp) != NULL && line_num < MAX_LINES) {
        input_data[line_num] = my_strdup(line_buffer);
        if (input_data[line_num] == NULL) {
            perror("Memory allocation failed for input_data");
            exit(1);
        }
        line_num++;
    }
    fclose(fp);
    printf("Successfully loaded %d lines of source code.\n\n", line_num);
}

void parse_source_code() {
    for (int i = 0; i < line_num; i++) {
        char *line_copy = my_strdup(input_data[i]);
        if (line_copy == NULL) continue;

        char *newline = strchr(line_copy, '\n');
        if (newline) *newline = '\0';

        token_table[i] = (token *)calloc(1, sizeof(token));
        if (token_table[i] == NULL) {
            perror("Memory allocation failed for token_table");
            exit(1);
        }

        if (line_copy[0] == '.') {
            strcpy(token_table[i]->comment, line_copy);
            free(line_copy); 
            continue; 
        }
        
        char *t1 = strtok(line_copy, " \t");
        if (t1 == NULL) { 
            free(line_copy);
            continue;
        }

        char *t2 = strtok(NULL, " \t");
        char *t3 = strtok(NULL, " \t");
        char *t4 = strtok(NULL, "\n"); 

        if (find_opcode(t1) == NULL && 
            strcmp(t1, "START") != 0 && strcmp(t1, "END") != 0 &&
            strcmp(t1, "RESW") != 0 && strcmp(t1, "RESB") != 0 &&
            strcmp(t1, "WORD") != 0 && strcmp(t1, "BYTE") != 0) 
        {
            token_table[i]->label = my_strdup(t1);
            if (t2) token_table[i]->operator = my_strdup(t2);
            if (t3) strcpy(token_table[i]->operand[0], t3); 
            if (t4) strcpy(token_table[i]->comment, t4);
        } else {
            token_table[i]->label = NULL;
            token_table[i]->operator = my_strdup(t1);
            if (t2) strcpy(token_table[i]->operand[0], t2); 
            if (t3) strcpy(token_table[i]->comment, t3);
            if (t4) strcat(token_table[i]->comment, t4); 
        }

        if (token_table[i]->operand[0][0] != '\0') {
            char *op_token = strtok(token_table[i]->operand[0], ",");
            int op_idx = 0;
            while(op_token != NULL && op_idx < MAX_OPERAND) {
                strcpy(token_table[i]->operand[op_idx], op_token);
                op_idx++;
                op_token = strtok(NULL, ",");
            }
            token_table[i]->operand_count = op_idx;
        }

        free(line_copy); 
    }
}

void print_parsed_output() {
    printf("--- SIC Program Listing (with OPCODE) ---\n");
    
    for (int i = 0; i < line_num; i++) {
        token *t = token_table[i];
        if (t == NULL) continue;
        
        char original_line[MAX_LINE_LENGTH];
        strcpy(original_line, input_data[i]);
        char *newline = strchr(original_line, '\n');
        if (newline) *newline = '\0';

        printf("%-5d %-30s", (i+1)*5, original_line);

        if (t->comment[0] != '\0' && t->operator == NULL) {
            printf("\n");
            continue;
        }

        if (t->operator != NULL) {
            inst *inst_info = find_opcode(t->operator);
            
            if (inst_info != NULL) {
                printf("OPCODE: %02X   ", inst_info->op);
                if (t->operand_count > 0) {
                    printf("OPERAND(s): %s", t->operand[0]);
                    if (t->operand_count > 1) {
                        printf(", %s", t->operand[1]);
                    }
                }
            } else {
                printf("(DIRECTIVE)");
            }
        }
        printf("\n");
    }
}

void free_all_memory() {
    for (int i = 0; i < inst_index; i++) {
        free(inst_table[i]);
    }

    for (int i = 0; i < line_num; i++) {
        free(input_data[i]); 
        if (token_table[i]) {
            free(token_table[i]->label);    
            free(token_table[i]->operator); 
            free(token_table[i]);         
        }
    }
}

char *my_strdup(const char *s) {
    if (s == NULL) return NULL;
    char *new_str = malloc(strlen(s) + 1);
    if (new_str == NULL) {
        perror("Memory allocation failed in my_strdup");
        exit(1);
    }
    strcpy(new_str, s);
    return new_str;
}
