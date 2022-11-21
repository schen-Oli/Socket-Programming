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
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <iostream>

#define IP "127.0.0.1"
#define PORT_M "25682" // PORT number of serverM
#define PORT_C "21682" // PORT numebr of serverC
#define MAXDATASIZE 100
#define BACKLOG 10

using namespace std;

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int getTcpSocketFd()
{
    struct addrinfo hints, *servinfo, *p;
    int sockfd;
    int yes = 1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int rv;
    if ((rv = getaddrinfo(IP, PORT_M, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1)
        {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "client: failed to connect\n");
        return -1;
    }

    freeaddrinfo(servinfo);
    return sockfd;
}

int varifyFromServerC(char *buf)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int numbytes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int rv;
    if ((rv = getaddrinfo(IP, PORT_C, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return false;
    }

    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("talker: socket");
            continue;
        }
        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "talker: failed to create socket\n");
        return 2;
    }

    if ((numbytes = sendto(sockfd, buf, strlen(buf), 0, p->ai_addr, p->ai_addrlen)) == -1)
    {
        perror("talker: sendto");
        exit(1);
    }

    char res[1];
    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof their_addr;
    if ((numbytes = recvfrom(sockfd, res, 1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
    {
        perror("recvfrom");
        exit(1);
    }

    freeaddrinfo(servinfo);

    close(sockfd);

    return res[0] - '0';
}

void encryption(char *buf)
{
    for (int i = 0; i < strlen(buf); i++)
    {
        char tmp = buf[i];
        if (isalpha(tmp))
        {
            int init;
            if (isupper(tmp))
            {
                init = 'A';
            }
            else
            {
                init = 'a';
            }

            buf[i] = (buf[i] - init + 4) % 26 + init;
        }
        else if (isdigit(tmp))
        {
            buf[i] = buf[i] - '0' + 4 >= 10 ? buf[i] + 4 - 10 : buf[i] + 4;
        }
    }
}

int auth(int new_fd)
{
    int numbytes;
    char buf[MAXDATASIZE];

    if ((numbytes = recv(new_fd, buf, MAXDATASIZE - 1, 0)) == -1)
    {
        perror("recv");
        close(new_fd);
        return false;
    }

    buf[numbytes] = '\0';
    encryption(buf);

    return varifyFromServerC(buf);
}

bool acceptConnections(int sockfd)
{
    struct sockaddr_storage their_addr;
    socklen_t sin_size = sizeof their_addr;
    int new_fd;
    char s[INET_ADDRSTRLEN];

    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

    if (new_fd == -1)
    {
        perror("accept");
        return false;
    }

    inet_ntop(their_addr.ss_family,
              get_in_addr((struct sockaddr *)&their_addr),
              s, sizeof s);
    printf("server: got connection from %s\n", s);

    if (!fork())
    {
        close(sockfd); // child doesn't need the listener

        // authentification loop
        while (1)
        {
            int sr;

            int authRet = auth(new_fd);
            if (authRet == 0)
            {
                sr = send(new_fd, "0", 1, 0);
                break;
            }
            else if (authRet == 1)
            {
                sr = send(new_fd, "1", 1, 0);
            }
            else if (authRet == 2)
            {
                sr = send(new_fd, "2", 1, 0);
            }
            else
            {
                perror("send");
                close(new_fd);
                exit(1);
            }
        }

        // request
        close(new_fd);
        exit(0);
    }

    close(new_fd); // parent doesn't need this
    return true;
}
