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
#define PORT "25682" // port number of serverM
#define MAXDATASIZE 100

using namespace std;

int getSolidSocketFd()
{
    struct addrinfo hints, *servinfo, *p;
    int sockfd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int rv;
    if ((rv = getaddrinfo(IP, PORT, &hints, &servinfo)) != 0)
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

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("client: connect");
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

void auth(int sockfd)
{
    int cnt = 3;

    while (cnt > 0)
    {
        cnt--;

        cout << "Please enter the username: ";
        string username;
        cin >> username;
        cout << "Please enter the password: ";
        string password;
        cin >> password;

        string data = "0" + username + "," + password;
        const char *msg = data.c_str();

        char buf[MAXDATASIZE];
        int numbytes;

        if (send(sockfd, msg, data.size(), 0) != data.size())
        {
            perror("send");
            exit(1);
        }

        cout << username << " sent an authentication request to the main server." << endl;

        if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
        {
            perror("recv");
            exit(1);
        }

        struct sockaddr_in sa;
        socklen_t sa_len = sizeof(sa);
        if (getsockname(sockfd, (struct sockaddr *)&sa, &sa_len) == -1)
        {
            perror("getsockname() failed");
            exit(1);
        }

        buf[numbytes] = '\0';

        if (buf[0] == '1')
        {
            printf("%s received the result of authentication using TCP over port %d. Authentication failed: Username Does not exist\n", username.c_str(), ntohs(sa.sin_port));
            printf("Attempts remaining:%d\n", cnt);
        }
        else if (buf[0] == '2')
        {
            printf("%s received the result of authentication using TCP over port %d. Authentication failed: Password does not match\n", username.c_str(), ntohs(sa.sin_port));
            printf("Attempts remaining:%d\n", cnt);
        }
        else
        {
            printf("%s received the result of authentication using TCP over port %d. Authentication is successful\n", username.c_str(), ntohs(sa.sin_port));
            return;
        }
    }

    close(sockfd);
    cout << "Authentication Failed for 3 attempts. Client will shut down." << endl;
    exit(1);
}
