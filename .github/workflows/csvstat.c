#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define max_fields_length 1024
#define max_fields 64

int max(int a, int b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

int parser(const char line[], char fields[max_fields][max_fields_length]) {

    int mode = 0; // Режим 0 - обычные поля, режим 1 - поля с кавычками
    int field_number = 0;
    int char_number = 0;
    int c = 0;

    for (; line[c] != '\0';) {

        if (line[c] == '"') {
            mode = 1;
        } else {
            mode = 0;
        }

        // Режим 0

        if (mode == 0) {
            for(; (line[c] != ','); c++) {
                if (line[c] == '\0') {
                    c--;
                    break;
                }
                fields[field_number][char_number] = line[c];
                char_number++;
            }

            fields[field_number][char_number] = '\0';

            while ((fields[field_number][char_number-1] == '\t' || fields[field_number][char_number-1] == ' ') && char_number != 0) {
                fields[field_number][char_number-1] = '\0';
                char_number--;
            }

            while (fields[field_number][0] == '\t' || fields[field_number][0] == ' ') {
                for (int i = 0; fields[field_number][i] != '\0'; i++) {
                    fields[field_number][i] = fields[field_number][i+1];
                    
                }
            }

            if (fields[field_number][0] == '\0') {
                fields[field_number][0] = 'N';
                fields[field_number][1] = 'U';
                fields[field_number][2] = 'L';
                fields[field_number][3] = 'L';
                fields[field_number][4] = '\0';
            }

            char_number = 0;
            field_number++;
            c++;
        }

        if (mode == 1) {
            c++;
            for(; ; c++) {
                if (line[c] == '"' && (line[c+1] == '\0' || line[c+1] == ',')) {
                    if (line[c+1] == '\0') {
                        c--;
                    }
                    break;
                }
                else if (line[c] == '"' && line[c+1] == '"') {
                    fields[field_number][char_number] = '"';
                    char_number++;
                    c++;
                } else {
                    fields[field_number][char_number] = line[c];
                    char_number++;
                }
            }

            fields[field_number][char_number] = '\0';
            char_number = 0;
            field_number++;
            c = c + 2;
        }
    }

    if (line[c-1] == ',') {
        fields[field_number][0] = 'N';
        fields[field_number][1] = 'U';
        fields[field_number][2] = 'L';
        fields[field_number][3] = 'L';
        fields[field_number][4] = '\0';
        field_number++;
    }

    return field_number;
}

int print_help() {
    printf("\nProgram handles .txt files with CSV information inside of it.");
    printf("\nRun program with ./csvstat schema <path-to-txt-file>");
    printf("\n");
}

int main(int argc, char *argv[]) {

    if (strcmp(argv[1], "--help") == 0 && argc == 2) {
        print_help();
        goto end;
    } else if (strcmp(argv[1], "schema") == 0 && argc == 3) {
    } else {
        printf("\033[31mError: Incorrect command.\033[0m\n");
        return 2;
    }


    FILE *file = fopen(argv[2], "r");
    if (!file) {
        fprintf(stderr, "\033[31mError: Couldn't open file!\033[0m\n");
        return 1;
    }

    char buffer[4096];
    char fields[max_fields][max_fields_length];
    int columns = 0;
    int column = -1;
    int types[64][1024] = {0};
    char headers[64][1024];

    printf("\n");

    while (fgets(buffer, sizeof(buffer), file)) {

        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }
        
        if (len == 0) {
            fprintf(stderr, "\033[31mError: Empty line.\033[0m\n");
            return 1;
        }

        int count = parser(buffer, fields);
        if (columns == 0) {
            columns = count;
        }

        column++;

        if (count < columns) {
            fprintf(stderr, "\033[31mError: Not enough columns in the line. Columns: %d, in the line: %d\033[0m\n", columns, count);
            return 1;
        }

        for (int i = 0; i < columns; i++) {

            int type = 0;

            if (column == 0) {
                strcpy(headers[i], fields[i]);
            }

            if (strcmp(fields[i], "NULL") == 0) {
                type = -1;
                goto set_type;
            }
            for(int j = 0; fields[i][j] != '\0'; j++) {
                if (fields[i][j+1] == '\0' && fields[i][j] == '.') {
                    type = 3;
                    goto set_type;
                }
                if (j == 0 && ((fields[i][j] != '+' && fields[i][j] != '-') && !isdigit(fields[i][j]))) {
                    type = 3;
                    goto set_type;
                }
                if (j > 0) {
                    if (!isdigit(fields[i][j]) && fields[i][j] != '.') {
                        type = 3;
                        goto set_type;
                    }
                }
            }

            for(int j = 0; fields[i][j] != '\0'; j++) {
                if (fields[i][j] == '.') {
                    type = 2;
                    goto set_type;
                }
            }

            type = 1;
            
            set_type:

            types[column][i] = type;
        }

        for (int j = 0; j < columns; j++) {
            printf("%d ", types[column][j]);
        }
        printf("\n");
    }

    if (strcmp(argv[1], "schema") == 0) {
        int maximum = 0;
        printf("rows %d\n", column+1);
        printf("columns %d\n", columns);
        for(int i = 0; i < columns; i++) {
            printf("%d \"%s\" ", i, headers[i]);
            maximum = 0;
            for(int j = 1; j < column; j++) {
                maximum = max(maximum, types[j][i]);
            }
            if (maximum == 0) {
                printf("NULL\n");
            }
            if (maximum == 1) {
                printf("INT\n");
            }
            if (maximum == 2) {
                printf("DOUBLE\n");
            }
            if (maximum == 3) {
                printf("STRING\n");
            }
        }
    }

    fclose(file);
    end:
    return 0;
}
