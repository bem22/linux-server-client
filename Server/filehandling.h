//
// Created by bem22 on 22/10/2019.
//
#include <stdio.h>

#ifndef SERVER_FILEHANDLING_H
#define SERVER_FILEHANDLING_H

#endif //SERVER_FILEHANDLING_H

int* init_line_no(int *ln, FILE *fp);
int get_line_number(FILE *fp);
int push_line(FILE *fp, char* line, int *i);
int has_multi_lines(FILE *fp);
