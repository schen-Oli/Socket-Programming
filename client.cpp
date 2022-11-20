#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include <signal.h>

#define SERVERC_PORT "21682" 
#define IP "127.0.0.1"

#define BACKLOG 10   // how many pending connections queue will hold

int main(void)
{
   
    struct addrinfo hints;
    struct addrinfo *servinfo, *i;
    int socketFD;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int status = getaddrinfo(IP, SERVERC_PORT, &hints, &servinfo);
    if(status != 0){
        fprintf(stderr, "getaddrinfo error: %s\n", status);
        exit(1);
    }

    for(i = servinfo; i != NULL; i = i->ai_next){
        socketFD = socket(i->ai_family, i->ai_socktype, i->ai_protocol);

        if(socketFD == -1){
            continue;
        }

        int bindRet = bind(socketFD, i->ai_addr, i->ai_addrlen);
        if(bindRet == -1){
            close(socketFD);
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (i == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    struct sockaddr_in sa;
    socklen_t sa_len = sizeof(sa);
    if (getsockname(socketFD, (struct sockaddr *)&sa, &sa_len) == -1) {
      perror("getsockname() failed");
      return -1;
   }

    printf("The ServerC is up and running using UDP on port %d\n", ntohs(sa.sin_port));
    close(socketFD);

    return 0;
}