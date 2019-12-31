//
// Created by bem22 on 22/10/2019.
//

#include <stdlib.h>
#include "filehandling.h"
#include <string.h>
#include <pthread.h>

int get_line_number(FILE *fp) {
    int offset =   1, ch;
    // Set the pointer to the end of file minus offset and increase the offset in return
    while (fp) {
        fseek(fp, -offset++, SEEK_END);

        ch = fgetc(fp);
        // When we find a newline, stop
        if (ch == '\n') {
            // Create a buffer with the size of the line after newline
            char buf[offset];

            // Read the line
            fgets(buf, offset, fp);

            // create iteration pointer
            char *ptr = buf;

            // go to the first fullstop
            while (*ptr != ' ') {
                ptr++;
            }

            // put the 0 character to limit the string
            *ptr = '\0';

            // return the number from string to int (delimited by '\0'
            return atoi(buf) + 1;
        }
    }

    return -1;
}

void push_line_helper(FILE *fileptr, char* line, int line_number, int nl) {
    if(line_number == 1) {
        fprintf(fileptr, "%d %s", line_number, line);
    }
    else {
        fprintf(fileptr, "\n%d %s", line_number, line);
    }
}

int has_multi_lines(FILE *fptr) {
    int c;
    int offset = 0;
    fseek(fptr, 0, SEEK_SET);
    c = fgetc(fptr);
    while(c != EOF) {
        c = fgetc(fptr);

        // Put fptr at BOF + offset
        fseek(fptr, offset++, SEEK_SET);

        if(c == '\n') { return 1; }
    }
    return 0;
}

int push_line(FILE *fp, char* line, int *ip) {
    if(!fp || !ip || !line) {
        fprintf(stderr, "Error appending to file");
        return -1;
    }

    push_line_helper(fp, line, *ip, 1);

    return 0;
}

int* init_line_no(int* line_no, FILE *fp) {

    if(!fp) { return NULL; }
    line_no = (int*) malloc(sizeof(int));

    if(!line_no) { return NULL; }
    // Set line_no
    if(has_multi_lines(fp)) {
        *line_no = get_line_number(fp);
    }
    else {
        *line_no = 1;
    }

    return line_no;
}