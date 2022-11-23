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

#define PORT "21682"
#define IP "127.0.0.1"
#define MAXBUFLEN 100
#define PASS '0'
#define FAIL_NO_USER '1'
#define FAIL_PASS_NO_MATCH '2'

using namespace std;

map<string, string> db;
int sockfd;

void readFile(string fileName)
{
    ifstream file;
    file.open(fileName);

    if (!file.is_open())
    {
        perror("failed to open credential file");
        exit(-1);
    }

    string line;
    while (getline(file, line))
    {
        stringstream ss(line);
        string username;
        string password;
        getline(ss, username, ',');
        getline(ss, password);
        password.erase(remove(password.begin(), password.end(), '\n'), password.cend());
        password.erase(remove(password.begin(), password.end(), '\r'), password.cend());
        db.insert(make_pair(username, password));
    }
}

void creatUDPConnection()
{
    struct addrinfo hints;
    int rv;
    struct addrinfo *p, *servinfo;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(IP, PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return;
    }

    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "listener: failed to bind socket\n");
        return;
    }

    freeaddrinfo(servinfo);
}

void checkMessage()
{
    int numbytes;
    char buf[MAXBUFLEN];
    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof their_addr;

    memset(buf, 0, sizeof buf);
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
    {
        perror("recvfrom");
        exit(1);
    }
    cout << "The ServerC received an authentication request from the Main Server." << endl;

    string msg(buf, strlen(buf));

    stringstream ss(msg);
    string username;
    string password;
    getline(ss, username, ',');
    getline(ss, password);

    cout << "username is "
         << "'" << username << "'" << endl;
    cout << "password is "
         << "'" << password << "'" << endl;

    char res[1];
    memset(res, 0, sizeof res);
    res[0] = PASS;
    if (!db.count(username))
    {
        res[0] = FAIL_NO_USER;
    }
    else if (db[username] != password)
    {
        res[0] = FAIL_PASS_NO_MATCH;
    }

    if ((numbytes = sendto(sockfd, res, 1, 0, (struct sockaddr *)&their_addr, addr_len)) == -1)
    {
        perror("talker: sendto");
        exit(1);
    }
    cout << "The ServerC finished sending the response to the Main Server." << endl;
}