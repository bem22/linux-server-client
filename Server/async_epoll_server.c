#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <errno.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
#define MAXEVENTS 64

//Global variables
static int socket_fd, epoll_fd;

static void socket_create_bind_local()
{
    // create sockaddr_in (server address)
    struct sockaddr_in server_addr;
    int opt = 1;

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket");
        exit(1);
    }

    if (setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(int)) == -1) {
        perror("Setsockopt");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5999);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    bzero(&(server_addr.sin_zero),8);

    // bind to port
    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))
        == -1) {
        perror("Unable to bind");
        exit(1);
    }
}

// Make server_socket fd non blocking
static int make_socket_non_blocking(int sfd)
{
    int flags;

    flags = fcntl(sfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl");
        return -1;
    }

    flags |= O_NONBLOCK;
    if (fcntl(sfd, F_SETFL, flags) == -1) {
        perror("fcntl");
        return -1;
    }

    return 0;
}

void accept_and_add_new()
{
    struct epoll_event event;
    struct sockaddr in_addr;
    socklen_t in_len = sizeof(in_addr);
    int infd;
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

    while ((infd = accept(socket_fd, &in_addr, &in_len)) != -1) {

        if (getnameinfo(&in_addr, in_len,
                        hbuf, sizeof(hbuf),
                        sbuf, sizeof(sbuf),
                        NI_NUMERICHOST | NI_NUMERICHOST) == 0) {
            printf("Accepted connection on descriptor %d (host=%s, port=%s)\n",
                   infd, hbuf, sbuf);
        }
        /* Make the incoming socket non-block
         * and add it to list of fds to
         * monitor*/
        if (make_socket_non_blocking(infd) == -1) {
            abort();
        }

        event.data.fd = infd;
        event.events = EPOLLIN | EPOLLET;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, infd, &event) == -1) {
            perror("epoll_ctl");
            abort();
        }
        in_len = sizeof(in_addr);
    }

    if (errno != EAGAIN && errno != EWOULDBLOCK)
        perror("accept");
    /* else
     *
     * We hae processed all incomming connectioins
     *
     */
}

void process_new_data(int fd)
{
    ssize_t count;

    // 32 bits for the number of bytes that we will receive
    char buf[256];

    char *zeroes = NULL;

    while (count = read(fd, buf, sizeof(buf)-1)) {
        if (count == -1) {
            /* EAGAIN, read all data */
            if (errno == EAGAIN)
                return;

            perror("read");
            break;
        }

        /* Write buffer to stdout */
        buf[count] = '\0';
        printf("%s \n", buf);
    }

    printf("Close connection on descriptor: %d\n", fd);
    close(fd);
}

int main()
{
    // Create array of events and declare event
    struct epoll_event event, *events;

    //Bind local socket to port and address
    socket_create_bind_local();

    // Make sockfd non blocking
    if (make_socket_non_blocking(socket_fd) == -1)
        exit(1);

    // Start listening with a queue of max 5
    if (listen(socket_fd, 5) == -1) {
        perror("Listen");
        exit(1);
    }

    // Now wait for clients
    printf("\nTCPServer Waiting for client on port 5000\n");
    fflush(stdout);

    // Open a file descriptor (creates an epoll instance)
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        exit(1);
    }

    event.data.fd = socket_fd;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event) == -1) {
        perror("epoll_ctl");
        exit(1);
    }

    events = calloc(MAXEVENTS, sizeof(event));

    while(1) {
        int n, i;
        n = epoll_wait(epoll_fd, events, MAXEVENTS, -1);
        for (i = 0; i < n; i++) {
            if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP ||
                !(events[i].events & EPOLLIN)) {
                /* An error on this fd or socket not ready */
                perror("epoll error");
                close(events[i].data.fd);
            } else if (events[i].data.fd == socket_fd) {
                /* New incoming connection */
                accept_and_add_new();
            } else {
                /* Data incoming on fd */
                process_new_data(events[i].data.fd);
            }
        }
    }

    free(events);
    close(socket_fd);
    return 0;
}

#pragma clang diagnostic pop