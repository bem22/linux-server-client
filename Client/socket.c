#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


// This is going to be IPv4 only
int create_sock() {
    int client_sock;

    client_sock = socket(AF_INET, SOCK_STREAM, 0);

    if(!client_sock) { return -1; }

    return client_sock;
}


int client_connect(struct sockaddr server_addr, int client_sock) {

    if(connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        return 0;
    }

    return 1;
}