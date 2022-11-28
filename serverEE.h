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

#define EE_IP "127.0.0.1"

#define EE_PORT "23682"
#define EE_MAXBUFLEN 100

#define EE_CREDIT "1"
#define EE_PROFESSOR "2"
#define EE_DAYS "3"
#define EE_COURSE_NAME "4"
#define EE_ALL "5"

using namespace std;

struct Course
{
    string code;
    string credit;
    string professor;
    string days;
    string name;
};

int sockfd = -1;
map<string, Course> db;

// Read EE courses to a hashmap given file name
void ReadFile()
{
    ifstream file;
    file.open("ee.txt", ios::in);

    if (!file.is_open())
    {
        printf("ServerEE:\n Failed to open EE file.\n");
        exit(-1);
    }

    string line;
    while (getline(file, line))
    {
        stringstream ss(line);
        string code, credit, professor, days, name;
        getline(ss, code, ',');
        getline(ss, credit, ',');
        getline(ss, professor, ',');
        getline(ss, days, ',');
        getline(ss, name, ',');
        // remove tailing '\r' and '\n'
        while(name[name.length() - 1] == '\r' || name[name.length() - 1] == '\n'){
            name.erase(name.length() - 1);
        }
        Course course = {code, credit, professor, days, name};
        db[code] = course;
    }
}

// Find a valid socket fd and bind it to the port 23682
// Refer to "Beej's Guide to Network Programming"
void creatUDPConnection()
{
    struct addrinfo hints;
    int rv;
    struct addrinfo *p, *servinfo;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(EE_IP, EE_PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "ServerEE:\n getaddrinfo: %s\n", gai_strerror(rv));
        return;
    }

    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if ((sockfd) == -1)
        {
            perror("ServerEE:\n socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("serverEE:\n bind");
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "serverEE: failed to bind socket\n");
        return;
    }

    freeaddrinfo(servinfo);
    printf("The serverEE is up and running using UDP on port %s.\n", EE_PORT);
}

// return all information of a course as a string
// eg. EE450: 4, Ali Zahid, Tue;Thu, Introduction to Computer Networks
string getAllInfo(Course course)
{
    return course.code + ": " + course.credit + ", " + course.professor + ", " + course.days + ", " + course.name;
}

string getStringFromCategory(string cat)
{
    if (cat == "1")
        return "credit";
    if (cat == "2")
        return "professor";
    if (cat == "3")
        return "days";
    if (cat == "4")
        return "course name";
    return NULL;
}

// Recieve request from serverM
// Check course information from map db
// Format and send response to serverM
void checkMessage()
{
    int numbytes;
    char buf[EE_MAXBUFLEN];
    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof their_addr;

    memset(buf, 0, sizeof buf);

    // Recieve request message from main sever
    if ((numbytes = recvfrom(sockfd, buf, EE_MAXBUFLEN - 1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
    {
        perror("checkMessage - recvfrom");
        return;
    }

    string msg(buf, strlen(buf));

    stringstream ss(msg);
    string code;
    string cat;
    getline(ss, code, ',');
    if(ss.peek() == EOF){
        cout << "ServerEE:\n incorrect input format." << endl;
        cout << "Expected input format: Coursecode,RequestType (eg. EE450,3)" << endl;
        return;
    }
    getline(ss, cat);

    if (cat != EE_ALL)
    {
        printf("The serverEE received a request from the Main Server about the %s of %s.\n", getStringFromCategory(cat).c_str(), code.c_str());
    }else{
        printf("The serverEE received a request from the Main Server about %s.\n", code.c_str());
    }

    string res;
    if (!db.count(code))
    {
        res = "1Didn’t find the course: " + code;
        cout << "Didn’t find the course: " << code << endl;
    }
    else
    {
        res = "2";
        Course course = db[code];
        string information;
        if (cat == EE_CREDIT)
        {
            information = course.credit;
        }
        else if (cat == EE_PROFESSOR)
        {
            information = course.professor;
        }
        else if (cat == EE_DAYS)
        {
            information = course.days;
        }
        else if (cat == EE_COURSE_NAME)
        {
            information = course.name;
        }
        else if (cat == EE_ALL)
        {
            information = getAllInfo(course);
        }

        res += information;
        if (cat != EE_ALL)
        {
            printf("The course information has been found: The %s of %s is %s.\n", getStringFromCategory(cat).c_str(), code.c_str(), information.c_str());
        }
    }

    if ((numbytes = sendto(sockfd, res.c_str(), res.length(), 0, (struct sockaddr *)&their_addr, addr_len)) == -1)
    {
        perror("ServerEE:\n sendto");
        return;
    }
    cout << "The serverEE finished sending the response to the Main Server." << endl;
}