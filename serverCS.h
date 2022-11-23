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
#define PORT "22682"
#define MAXBUFLEN 100

// Credit / Professor / Days / CourseName / ALL
#define CREDIT "1"
#define PROFESSOR "2"
#define DAYS "3"
#define COURSE_NAME "4"
#define ALL "5"

using namespace std;

int sockfd;

struct Course
{
    string code;
    string credit;
    string professor;
    string days;
    string name;
};

map<string, Course> db;

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
        string code, credit, professor, days, name;
        getline(ss, code, ',');
        getline(ss, credit, ',');
        getline(ss, professor, ',');
        getline(ss, days, ',');
        getline(ss, name, ',');
        Course course = {code, credit, professor, days, name};
        db[code] = course;
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
            perror("serverCS, socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("serverCS, bind");
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "serverCS: failed to bind socket\n");
        return;
    }

    freeaddrinfo(servinfo);
    cout << "The ServerCS is up and running using UDP on port " << PORT << "." << endl;
}

string getAllInfo(Course course)
{
    string ret;
    ret = course.code + ": " + course.credit + ", " + course.professor + ", " + course.days + ", " + course.name;
    return ret;
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

    string msg(buf, strlen(buf));

    stringstream ss(msg);
    string code;
    string cat;
    getline(ss, code, ',');
    getline(ss, cat);

    if(cat != ALL){
        cout << "The ServerCS received a request from the Main Server about the " << getStringFromCategory(cat) << " of " << code << "." << endl;
    }

    string res;
    if (!db.count(code))
    {
        res = "0";
        cout << "Didn’t find the course: " << code << endl;
    }
    else
    {
        res = "1";
        Course course = db[code];
        string information;
        if (cat == CREDIT)
        {
            information = course.credit;
        }
        else if (cat == PROFESSOR)
        {
            information = course.professor;
        }
        else if (cat == DAYS)
        {
            information = course.days;
        }
        else if (cat == COURSE_NAME)
        {
            information = course.name;
        }
        else if (cat == ALL)
        {
            information = getAllInfo(course);
        }

        res += information;
        if(cat != ALL){
            cout << "The course information has been found: The " << getStringFromCategory(cat) << " of " << code << " is " << information << "." << endl;
        }
    }

    if ((numbytes = sendto(sockfd, res.c_str(), res.length(), 0, (struct sockaddr *)&their_addr, addr_len)) == -1)
    {
        perror("talker: sendto");
        exit(1);
    }
    cout << "The ServerCS finished sending the response to the Main Server." << endl;
}