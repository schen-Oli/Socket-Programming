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
#include <ctype.h>

#define IP "127.0.0.1"
#define PORT "25682" // port number of serverM
#define MAXDATASIZE 100

// EE / CS / OTHER
#define EE '1'
#define CS '2'
#define OTHER '3'

// Credit / Professor / Days / CourseName
#define CREDIT '1'
#define PROFESSOR '2'
#define DAYS '3'
#define COURSE_NAME '4'
#define FULL '5'

using namespace std;

string username;

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
        cin >> username;
        cout << "Please enter the password: ";
        string password;
        cin >> password;

        string data = username + "," + password;
        const char *msg = data.c_str();

        char buf[MAXDATASIZE];
        int numbytes;
        cout << "data size: " << data.size() << endl;
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

// char getDepartment(string dep)
// {
//     if (dep == "EE")
//     {
//         return EE;
//     }
//     else if (dep == "CS")
//     {
//         return CS;
//     }
//     return OTHER;
// }

char getCat(string category)
{
    if (category == "Credit")
    {
        return CREDIT;
    }
    else if (category == "Professor")
    {
        return PROFESSOR;
    }
    else if (category == "Days")
    {
        return DAYS;
    }
    return COURSE_NAME;
}

void requestOneCourse(string req, string code, int sockfd)
{
    cout << "Please enter the category (Credit / Professor / Days / CourseName): ";
    string category;
    cin >> category;

    string dep;
    string courseCode;

    int index = 0;

    req += code + ",";
    req += getCat(category);

    const char *msg = req.c_str();

    cout << "sending '" << req << "'" << endl;

    int numbytes;
    if (send(sockfd, msg, req.size(), 0) != req.size())
    {
        perror("send");
        exit(1);
    }

    cout << username << " sent a request to the main server." << endl;

    char buf[MAXDATASIZE];
    memset(buf, 0, sizeof buf);
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
    {
        perror("recv");
        exit(1);
    }

    string resposne = string(buf, strlen(buf));
    
    cout << "The " << category << " of " << code << " is " << buf << endl;  
}

void requestMultiCourse(string req, string code, int sockfd)
{
    int index = 0;
    while (index < code.length())
    {
        if (code[index] == ' ')
        {
            req.append(" ");
            while (index < code.length() && code[index] == ' ')
            {
                index++;
            }
        }

        if (isalpha(code[index]))
        {
            string dep;
            while (index < code.length() && isalpha(code[index]))
            {
                dep += code[index];
                index++;
            }
            req += dep;
        }

        if (isdigit(code[index]))
        {
            req += code[index];
            index++;
        }
    }

    const char *msg = req.c_str();

    int numbytes;
    if (send(sockfd, msg, req.size(), 0) != req.size())
    {
        perror("send");
        exit(1);
    }

    cout << username << " sent a request to the main server." << endl;
}

string trim(string input)
{
    int start = 0;
    int end = input.length() - 1;
    while (input[start] == ' ')
    {
        start++;
    }
    while (input[end] == ' ')
    {
        end--;
    }
    return input.substr(start, end - start + 1);
}

void requestCourse(int sockfd)
{
    cout << "Please enter the course code to query: ";
    string code;
    getline(std::cin >> std::ws, code);

    code = trim(code);

    string request;

    if (code.find_first_of(' ', 0) == string::npos)
    {
        // only request for one course
        request.append("1");
        requestOneCourse(request, code, sockfd);
    }
    else
    {
        // request for multiple courses
        request.append("2");
        requestMultiCourse(request, code, sockfd);
    }
}
