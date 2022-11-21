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

using namespace std;

map<string, string> db;
int sockfd;
struct addrinfo *p, *servinfo;

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
        db.insert(make_pair(username, password));
    }
}

void creatUDPConnection()
{
    struct addrinfo hints;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
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
}

void checkMessage()
{
    int numbytes;
    char buf[MAXBUFLEN];
    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof their_addr;

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

    char res[1];
    res[0] = '0';
    if (!db.count(username))
    {
        res[0] = '1';
    }else if (db[username] != password)
    {
        res[0] = '2';
    }
    
    if ((numbytes = sendto(sockfd, res, 1, 0, (struct sockaddr *)&their_addr, addr_len)) == -1)
    {
        perror("talker: sendto");
        exit(1);
    }
    cout << "The ServerC finished sending the response to the Main Server." << endl;

}