#include<stdlib.h>
#include<stdio.h>
#include<string.h>

void freen(void * p) {
    free(p);
    p = NULL;
}

// Function to read a dynamically allocated string. It can read characters a much as memory allows
char* read_string() {
    size_t incrementStep = 20;
    size_t maxLength = incrementStep;

    int character;

    // Allocate memory to the buffer with the size of incrementStep
    char *buffer = (char*) malloc(incrementStep);
    char *currentPosition = buffer; 
    
    // Initial string length is 0
    size_t length = 0;

    // If buffer is NULL, memory could not be allocated
    if(!buffer) {
        return NULL;
    }
    
    while (1) {
        character = fgetc(stdin);

        if(character =='\n'){break;}
        if(feof(stdin)) {
            break;
        }

        if(++length >= maxLength) {
            maxLength += incrementStep;
            char *newBuffer = (char*)realloc(buffer, maxLength);

            if(!newBuffer) {
                free(buffer);
                return NULL;
            }

            currentPosition = newBuffer + (currentPosition - buffer);   
            buffer = newBuffer;            
        }
        *currentPosition = character;
        currentPosition ++;
    }

    *currentPosition = '\0';
    return buffer;
}

// This should modify *p
char* add_size(char* p) {
    char buffer[100] = {0};
    unsigned int string_length = strlen(p);
    sprintf(buffer,"%d", string_length);
    size_t size_string_format_size = 3 + strlen(buffer);

    char* size_string_format = (char*) malloc(size_string_format_size + 1);
    if(!size_string_format) {
        return NULL;
    }
    sprintf(size_string_format, "%d%s", string_length, "BEG");

    char* result = (char*) malloc(string_length + size_string_format_size + 1);
    if(!result) {
        free(size_string_format);
        return NULL;
    }

    sprintf(result, "%s%s", size_string_format, p);

    // Free the memory
    freen(size_string_format);
    free(p);
    return result;
}
