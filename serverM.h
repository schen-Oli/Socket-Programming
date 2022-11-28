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
#define PORT_C "21682"
#define PORT_CS "22682"
#define PORT_EE "23682"
#define PORT_M "25682"
#define PORT_M_UDP "24682"

#define MAXDATASIZE 100
#define SHIFT_ENCRYP 4

#define CREDIT '1'
#define PROFESSOR '2'
#define DAYS '3'
#define COURSE_NAME '4'
#define FULL '5'

using namespace std;

string username;

// give the category in string format and return it's code
string getCat(string category)
{
    if (category == "1")
    {
        return "credit";
    }
    else if (category == "2")
    {
        return "professor";
    }
    else if (category == "3")
    {
        return "days";
    }
    else if (category == "4")
    {
        return "course name";
    }
    return "all";
}

// get the port number given socket fd
// Refer to the assignment description
uint16_t getPortNumber(int sockfd)
{
    struct sockaddr_in sa;
    socklen_t sa_len = sizeof(sa);
    if (getsockname(sockfd, (struct sockaddr *)&sa, &sa_len) == -1)
    {
        perror("serverM:\n getsockname() failed");
        return 0;
    }

    return ntohs(sa.sin_port);
}

// Find a valid socket fd and bind it to the port 25682
// Refer to "Beej's Guide to Network Programming"
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

// create a UDP socket to serverC
// Send username and password to serverC throught the UDP socket 
// return response as integer
int varifyFromServerC(char *buf)
{
    // Refer to "Beej's Guide to Network Programming"
    int sockfd;
    struct addrinfo hints, *servinfoM, *servinfoC, *p;
    int numbytes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int rv;
    if ((rv = getaddrinfo(IP, PORT_M_UDP, &hints, &servinfoM)) != 0)
    {
        fprintf(stderr, "varifyFromServerC - getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    for (p = servinfoM; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("varifyFromServerC - socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("varifyFromServerC - bind");
            continue;
        }

        break;
    }

    if ((rv = getaddrinfo(IP, PORT_C, &hints, &servinfoC)) != 0)
    {
        fprintf(stderr, "varifyFromServerC - getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    if (p == NULL)
    {
        fprintf(stderr, "varifyFromServerC - failed to create socket\n");
        return -1;
    }

    if ((numbytes = sendto(sockfd, buf, strlen(buf), 0, servinfoC->ai_addr, servinfoC->ai_addrlen)) == -1)
    {
        perror("varifyFromServerC - sendto");
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

    printf("The main server received the result of the authentication request from ServerC using UDP over port %d.\n", getPortNumber(sockfd));
    freeaddrinfo(servinfoM);
    freeaddrinfo(servinfoC);
    close(sockfd);

    return res[0] - '0';
}

// offset each letter or number by 4
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

// Recieve authentication request from Client
// Return authentication result as integer
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

    // if buf[0] == '0', the user wants to end connection
    if (buf[0] == '0')
    {
        close(new_fd);
        exit(0);
    }

    username.clear();
    int index = 1;

    while (index < numbytes && buf[index] != ',')
    {
        username += buf[index++];
    }

    printf("The main server received the authentication for %s using TCP over port %d.\n", username.c_str(), getPortNumber(new_fd));

    encryption(buf + 1);

    return varifyFromServerC(buf + 1);
}

// create a UDP socket to serverEE / serverCS
// Send reqeust to serverEE / serverCS throught the UDP socket 
// return response as a string
string getInfoFromServer(const char *port, string department, string req)
{
    // Refer to "Beej's Guide to Network Programming"
    int sockfd;
    struct addrinfo hints, *servinfoM, *servinfoX, *p;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int rv;
    if ((rv = getaddrinfo(IP, PORT_M_UDP, &hints, &servinfoM)) != 0)
    {
        fprintf(stderr, "getInfoFromServer - getaddrinfo: %s\n", gai_strerror(rv));
        return NULL;
    }

    for (p = servinfoM; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("getInfoFromServer - failed to create socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("getInfoFromServer - bind");
            continue;
        }
        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "getInfoFromServer - failed to create socket\n");
        return NULL;
    }

    if ((rv = getaddrinfo(IP, port, &hints, &servinfoX)) != 0)
    {
        fprintf(stderr, "getInfoFromServer - getaddrinfo: %s\n", gai_strerror(rv));
        return NULL;
    }

    int numbytes;
    if ((numbytes = sendto(sockfd, req.c_str(), req.length(), 0, servinfoX->ai_addr, servinfoX->ai_addrlen)) == -1)
    {
        perror("ServerM: sendto");
        exit(1);
    }

    printf("The main server sent a request to server%s.\n", department.c_str());

    char res[MAXDATASIZE];
    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof their_addr;
    if ((numbytes = recvfrom(sockfd, res, MAXDATASIZE, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
    {
        perror("recvfrom");
        exit(1);
    }

    freeaddrinfo(servinfoM);
    freeaddrinfo(servinfoX);
    res[numbytes] = '\0';

    printf("The main server received the response from server%s using UDP over port %d.\n", department.c_str(), getPortNumber(sockfd));

    close(sockfd);
    return string(res, strlen(res));
}

// process request for one course information
int processOneCourse(char *buf, int new_fd)
{
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
        perror("wrong request format");
        return -1;
    }

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
        perror("wrong request format");
        return -1;
    }

    if (buf[index] == ',')
    {
        category += buf[index + 1];
    }
    else
    {
        perror("wrong request format");
        return -1;
    }

    printf("The main server received from %s to query course %s about %s using TCP over port %d.\n", username.c_str(), (department + courseCode).c_str(), getCat(category).c_str(), getPortNumber(new_fd));

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
        res = "1";
    }
    sr = send(new_fd, res.c_str(), res.length(), 0);
    cout << "The main server sent the query information to the client." << endl;

    return sr;
}

// process request for multiple course information
int processMultipleCourses(char *buf, int new_fd)
{
    string department;
    string courseCode;

    int index = 1;
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
        perror("wrong request format");
        return -1;
    }

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
        perror("wrong request format");
        return -1;
    }

    string res;
    if (department == "EE")
    {
        res = getInfoFromServer(PORT_EE, "EE", department + courseCode + ",5");
    }
    else if (department == "CS")
    {
        res = getInfoFromServer(PORT_CS, "CS", department + courseCode + ",5");
    }
    else
    {
        res = "1Didn’t find the course: " + department + courseCode;
    }

    int sr = send(new_fd, res.c_str(), res.length(), 0);
    return sr;
}

// recieve request from client
// determine whether its a single course request or multiple course request
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
    else if (buf[0] == '2')
    {
        processMultipleCourses(buf, new_fd);
    }
    else
    {
        close(new_fd);
        exit(1);
    }
}

void acceptConnections(int new_fd)
{
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
        if (sr == -1)
        {
            perror("send");
            close(new_fd);
            return;
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
