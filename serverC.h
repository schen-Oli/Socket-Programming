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

void readFile()
{
    ifstream file;
    file.open("cred.txt", ios::in);

    if (!file.is_open())
    {
        perror("serverC: failed to open credential file");
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
        // remove tailing '\r' and '\n'
        while(password[password.length() - 1] == '\r' || password[password.length() - 1] == '\n'){
            password.erase(password.length() - 1);
        }
        db.insert(make_pair(username, password));
    }
}

// Find a valid socket fd and bind the port to it
// Code refers to "Beej's Guide to Network Programming"
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
            perror("serverC: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("serverC: bind");
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "serverC: failed to bind socket\n");
        return;
    }

    freeaddrinfo(servinfo);
}

// Recieve authentication message from ServerM
// Check username and password against map db
// Send response to ServerM
void checkMessage()
{
    int numbytes;
    char buf[MAXBUFLEN];
    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof their_addr;

    memset(buf, 0, sizeof buf);
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
    {
        perror("serverC: recvfrom");
        return;
    }
    cout << "The ServerC received an authentication request from the Main Server." << endl;

    string msg(buf, strlen(buf));

    stringstream ss(msg);
    string username;
    string password;
    getline(ss, username, ',');
    getline(ss, password);

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
        perror("ServerC: sendto");
        return;
    }
    cout << "The ServerC finished sending the response to the Main Server." << endl;
}