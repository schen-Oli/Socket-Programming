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
#include <vector>
#define IP "127.0.0.1"

#define PORT_M "25682"
#define PORT_C "21682"
#define PORT_CS "22682"
#define PORT_EE "23682"

#define MAXDATASIZE 100
#define BACKLOG 10
#define SHIFT_ENCRYP 4

using namespace std;

string username;

// Find a valid socket fd and bind it to the address
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

// Send username and password to serverC
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
    else
    {
        cout << "The main server sent an authentication request to serverC." << endl;
    }

    char res[1];
    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof their_addr;
    if ((numbytes = recvfrom(sockfd, res, 1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
    {
        perror("recvfrom");
        exit(1);
    }
    else
    {
        struct sockaddr_in sa;
        socklen_t sa_len = sizeof(sa);
        if (getsockname(sockfd, (struct sockaddr *)&sa, &sa_len) == -1)
        {
            perror("getsockname() failed");
            exit(1);
        }

        printf("The main server received the result of the authentication request from ServerC using UDP over port %d.\n", ntohs(sa.sin_port));
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

            buf[i] = (buf[i] - init + SHIFT_ENCRYP) % 26 + init;
        }
        else if (isdigit(tmp))
        {
            buf[i] = buf[i] - '0' + SHIFT_ENCRYP >= 10 ? buf[i] + SHIFT_ENCRYP - 10 : buf[i] + SHIFT_ENCRYP;
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

    username.clear();
    int index = 1;

    while (index < numbytes && buf[index] != ',')
    {
        username += buf[index++];
    }

    // if buf[0] == '1', the user wants to end connection
    if (buf[0] == '1')
    {
        close(new_fd);
        exit(0);
    }

    struct sockaddr_in sa;
    socklen_t sa_len = sizeof(sa);
    if (getsockname(new_fd, (struct sockaddr *)&sa, &sa_len) == -1)
    {
        perror("getsockname() failed");
        exit(1);
    }
    printf("The main server received the authentication for %s using TCP over port %d.\n", username.c_str(), ntohs(sa.sin_port));

    encryption(buf + 1);

    return varifyFromServerC(buf + 1);
}

string getInfoFromServer(const char *port, string department, string req)
{
    printf("The main server sent a request to server%s.\n", department.c_str());

    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int numbytes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int rv;
    if ((rv = getaddrinfo(IP, port, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return NULL;
    }

    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("serverM: failed to create socket");
            continue;
        }
        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "talker: failed to create socket\n");
        return NULL;
    }

    if ((numbytes = sendto(sockfd, req.c_str(), req.length(), 0, p->ai_addr, p->ai_addrlen)) == -1)
    {
        perror("talker: sendto");
        exit(1);
    }

    char res[MAXDATASIZE];
    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof their_addr;
    if ((numbytes = recvfrom(sockfd, res, MAXDATASIZE, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
    {
        perror("recvfrom");
        exit(1);
    }

    freeaddrinfo(servinfo);
    res[numbytes] = '\0';

    struct sockaddr_in sa;
    socklen_t sa_len = sizeof(sa);
    if (getsockname(sockfd, (struct sockaddr *)&sa, &sa_len) == -1)
    {
        perror("getsockname() failed");
        exit(1);
    }

    printf("The main server received the response from server%s using UDP over port %d.\n", department.c_str(), ntohs(sa.sin_port));
    
    close(sockfd);
    return string(res + 1, strlen(res));
}

int processOneCourse(char *buf, int new_fd)
{
    cout << "processing one requst " << string(buf, strlen(buf)) << endl;

    int index = 1;
    string department;
    string courseCode;
    string category;

    if (isalpha(buf[index]))
    {
        while (index < strlen(buf) && isalpha(buf[index]))
        {
            department += buf[index];
            index++;
        }
    }
    else
    {
        cout << "char in index " << index << " is " << buf[index];
        perror("wrong request format");
        return -1;
    }

    cout << "department is " << department << endl;

    if (isdigit(buf[index]))
    {
        while (index < strlen(buf) && isdigit(buf[index]))
        {
            courseCode += buf[index];
            index++;
        }
    }
    else
    {
        cout << "char in index " << index << " is " << buf[index];
        perror("wrong request format");
        return -1;
    }

    cout << "courseCode is " << courseCode << endl;

    if (buf[index] == ',')
    {
        category += buf[index + 1];
    }
    else
    {
        cout << "char in index " << index << " is " << buf[index];
        perror("wrong request format");
        return -1;
    }

    struct sockaddr_in sa;
    socklen_t sa_len = sizeof(sa);
    if (getsockname(new_fd, (struct sockaddr *)&sa, &sa_len) == -1)
    {
        perror("getsockname() failed");
        exit(1);
    }

    printf("The main server received from %s to query course %s about <category> using TCP over port %d.", username.c_str(), (department + courseCode).c_str(), ntohs(sa.sin_port));

    string req = courseCode + "," + category;
    int sr;
    string res;
    if (department == "EE")
    {
        res = getInfoFromServer(PORT_EE, "EE", "EE" + req);
    }
    else if (department == "CS")
    {
        res = getInfoFromServer(PORT_CS, "CS", "CS" + req);
    }
    else
    {
        res = "0Course code not exist";
    }
    sr = send(new_fd, res.c_str(), res.length(), 0);
    cout << "The main server sent the query information to the client." << endl;
    return sr;
}

int processMultipleCourses(char *buf, int new_fd)
{
    int left = 1, right = 1;
    string department;
    string courseCode;
    while (right < strlen(buf))
    {
        while (right < strlen(buf) && buf[right] != ' ')
        {
            right++;
        }

        string req(buf + left, right - left);
        string res;
        if (req.find("EE") != string::npos)
        {
            res = "1" + getInfoFromServer(PORT_EE, "EE", req + ",5");
        }
        else if (req.find("CS") != string::npos)
        {
            res = "1" + getInfoFromServer(PORT_CS, "CS", req + ",5");
        }
        else
        {
            res = "0Course code not exist";
        }

        int sr = send(new_fd, res.c_str(), res.length(), 0);
        right++;
        left = right;
    }

    cout << "finish sending all ans, send terminating message" << endl;
    send(new_fd, "0", 1, 0);
    return 0;
};

void processRequest(int new_fd)
{
    int numbytes;
    char buf[MAXDATASIZE];
    memset(buf, 0, sizeof buf);
    if ((numbytes = recv(new_fd, buf, MAXDATASIZE - 1, 0)) == -1)
    {
        perror("recv");
        close(new_fd);
        exit(1);
    }

    buf[numbytes] = '\0';

    if (buf[0] == '1')
    {
        processOneCourse(buf, new_fd);
    }
    else
    {
        processMultipleCourses(buf, new_fd);
    }
}

bool acceptConnections(int sockfd)
{
    struct sockaddr_storage their_addr;
    socklen_t sin_size = sizeof their_addr;

    int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

    if (new_fd == -1)
    {
        perror("accept");
        return false;
    }

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
                cout << "The main server sent the authentication result to the client." << endl;
                break;
            }
            else if (authRet == 1)
            {
                sr = send(new_fd, "1", 1, 0);
                cout << "The main server sent the authentication result to the client." << endl;
            }
            else if (authRet == 2)
            {
                sr = send(new_fd, "2", 1, 0);
                cout << "The main server sent the authentication result to the client." << endl;
            }
            else
            {
                perror("send");
                close(new_fd);
                exit(1);
            }
        }

        // get request
        while (1)
        {
            processRequest(new_fd);
        }

        close(new_fd);
        exit(0);
    }

    close(new_fd); // parent doesn't need this
    return true;
}
