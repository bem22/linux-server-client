#include "filehandling.h"
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

typedef struct client {
    int socket;
    struct sockaddr address;
    pthread_t *thrd;
} Client;

FILE *fp;
int *line_no = NULL;
int thread_counter = 0;
int max_no_threads = 15;
pthread_mutex_t file_mut;
void *filewrit(void *param) {
    // Reconvert the object from void pointer
    Client *client = param;
    char buf[1];
    char* init_buffer = NULL;

    char* buffer = NULL;
    char* message = NULL;
    // Check if the connection is still alive. while alive... do
    while (recv(client->socket, &buf,1, MSG_PEEK | MSG_DONTWAIT) != 0) {

        // Declare, allocate and memset the buffer and its size

        size_t init_buffer_size = 100;
        size_t buffer_size = 0;

        init_buffer = (char*) malloc(init_buffer_size + 1);
        if(!init_buffer) {
            return NULL;
        }
        memset(init_buffer, 0,  init_buffer_size + 1);

        // Variable to hold the number of bytes - return value of recv
        size_t recv_length = 0;

        recv_length = recv(client->socket, init_buffer, init_buffer_size, 0);

        if(recv_length == 0) {
            free(init_buffer);
            break; // Client died
        }

        /* A chunk is a part of the sent message from the client
         * A hunk is a message without the header
         * Sometimes a hunk may be just a chunk
         * Many hunks form a chunk
         */
        size_t chunk_length = 0;
        size_t message_size = 0;
        int header_shift_size = 0;

    // HEADER EXTRACTION
        // Calculate number of bytes left in the message
        for (int BEG_tracker = 0; BEG_tracker <= (int) recv_length; BEG_tracker++) {
            if (init_buffer[BEG_tracker] == 'B') {
                header_shift_size = 3 + BEG_tracker;

            // HEADER CONVERSION BEGIN
                // Read the header and put it in an int
                char* no_bytes_str = (char*) calloc(1,BEG_tracker + 1);
                if(!no_bytes_str) {
                    fprintf(stderr, "NOMEM");
                    free(init_buffer);
                    return NULL;
                }
                *(no_bytes_str + BEG_tracker) = '\0';

                memcpy(no_bytes_str, init_buffer, BEG_tracker);

                // Convert string from header to int to read no of bytes
                chunk_length = strtol(no_bytes_str, NULL, 10);

                // Flush the the string that held the number of bytes
                bzero(no_bytes_str, BEG_tracker);
                free(no_bytes_str);
                break;
            }
        }


        // SWAP BUFFERS BEGIN
        /* Swap the buffer and put the content in the
         * new one, getting rid of HEADER
         */

        if(chunk_length < 90) {
            message_size = chunk_length;
        } else {
            message_size = init_buffer_size - header_shift_size;
        }

        message = (char*) malloc(message_size);
        if(!message) {
            free(init_buffer);
            fprintf(stderr, "NOMEM");
            return NULL;
        }
        memcpy(message, &init_buffer[header_shift_size], message_size);

    // HEADER EXTRACTION FINISHED


        /* If the initial buffer was not enough for one message, we keep
         * reading and adding to the message
         */
        buffer_size = chunk_length - message_size;
        while (chunk_length > message_size) {
            // Extend buffer to read
            buffer = (char*) realloc(buffer, buffer_size + 1);

            if(!buffer) {
                bzero(message, message_size);
                free(message);
                fprintf(stderr, "NOMEM");
                return NULL;
            }

            buffer[buffer_size] = '\0';

            // Read the next value in buffer
            recv_length = recv(client->socket, buffer, buffer_size, 0);

            // Dead client?
            if(recv_length <= 0) { break; /* set some flag */ }

            // Expand message size to concat buffer
            message_size += recv_length;

            // Finally concat the buffer to message
            message = (char*) realloc(message, message_size + 1);
            memset(&message[message_size-recv_length], 0, recv_length);

            strcat(message, buffer);

            // Flush buffer
            bzero(buffer, buffer_size);

            // Create the new buffer_size
            buffer_size = chunk_length - message_size;
        }

        message = realloc(message, message_size + 1);
        // Make sure we stop the string.
        message[message_size] = '\0';

        if(strlen(message) == chunk_length) {
            // Finally write the message
            fflush(fp);
            push_line(fp, message, line_no);
            fflush(fp);
        }

        // Increment line number <CRITICAL SECTION>
        pthread_mutex_lock(&file_mut);
        (*line_no)++;
        pthread_mutex_unlock(&file_mut);

        // cleanup
        bzero(message, message_size);
        free(init_buffer);
        free(message);
    }

    // If the client failed, we free.
    if(buffer) { free(buffer); }
    // Free the thread pointer
    free(client->thrd);
    printf("Client %d went away :(\n", client->socket);
    free(client);
    return NULL;
}

int main(int argc, char *argv[]) {
    int port;
    char address[16];

    // Arguments handling
    if(argc != 2) {
        fprintf(stdout, "No params provided, assuming:localhost, port:5000\n");
        strcpy(address, "127.0.0.1");
        port = 5000;
    } else {
        if(strlen(argv[0]) <= 16) {
            strcpy(address, argv[0]);
            port = (int) strtol(argv[1], NULL, 10);
        }
        else {
            fprintf(stderr, "Bad args\n");
            return 1;
        }
    }

    fp = fopen("./hello.txt", "a+r");
    // Critical variable
    line_no = init_line_no(line_no, fp);

    struct sockaddr_in server_addr;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(address);
    server_addr.sin_family = AF_INET;
    bzero(&(server_addr.sin_zero),8);

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);

    fflush(stdin);

    if(bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == 0) {
        printf("%s", "Server bound on port 5000\n");
    } else {
        fprintf(stderr, "Unable to bind to the specified port\n");
        return 1;
    }

    if(listen(server_sock, 5) == -1) {
        printf("listen error");
    }

    int cfd;
    struct sockaddr caddr;
    socklen_t in_size = sizeof(caddr);


    printf("Ready to accept connections\n");
    while(1) {
        // Create client fds, the main thread will be busy here waiting for clients
        if ((cfd = accept(server_sock, &caddr, &in_size)) > -1) {
            printf("%s", "client connected\n");
            //if(thread_counter < max_no_threads) {
                // Create a new client object from cfd+struct
                Client *client = (Client*) malloc(sizeof(Client));
                //Save the stack values in the struct (cfd and caddr get rewritten)
                client->socket = cfd;
                client->address = caddr;

                // Create a thread as a pointer and save it to the list of threads
                pthread_t *worker = (pthread_t*) malloc(sizeof(pthread_t));
                thread_counter++;
                client->thrd = worker;

                // Start the thread
                pthread_create(worker, NULL, filewrit, client);
                pthread_join(*worker, NULL);
            //} else {
            //    close(cfd);
            //    printf("Max thread no reached");
            //}
        }
    }

    // Not entirely necessary now, because main thread is in a while(1)
    // Same as joining the worker threads
    free(line_no);
    line_no = NULL;
    fclose(fp);
    return 0;
}
