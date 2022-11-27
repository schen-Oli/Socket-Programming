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

#define CREDIT "1"
#define PROFESSOR "2"
#define DAYS "3"
#define COURSE_NAME "4"
#define ALL "5"

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

void readFile(string fileName)
{
    ifstream file;
    file.open(fileName);

    if (!file.is_open())
    {
        perror("ServerCS:\n Failed to open CS course file");
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
        while(name[name.length() - 1] == '\r' || name[name.length() - 1] == '\n'){
            name.erase(name.length() - 1);
        }
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
        fprintf(stderr, "ServerCS:\n getaddrinfo: %s\n", gai_strerror(rv));
        return;
    }

    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            perror("ServerCS:\n socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("ServerCS:\n bind");
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

    // Recieve request message from main sever
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
    {
        perror("ServerCS:\n recvfrom");
        return;
    }

    string msg(buf, strlen(buf));

    stringstream ss(msg);
    string code;
    string cat;
    getline(ss, code, ',');
    if(ss.peek() == EOF){
        cout << "ServerCS:\n incorrect input format." << endl;
        cout << " Expected input format: Coursecode,RequestType (eg. CS450,3)" << endl;
        return;
    }
    getline(ss, cat);

    if(cat != ALL){
        printf("The serverCS received a request from the Main Server about the %s of %s.\n", getStringFromCategory(cat).c_str(), code.c_str());
    }else{
        printf("The serverCS received a request from the Main Server about %s.\n", code.c_str());
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
            printf("The course information has been found: The %s of %s is %s.\n", getStringFromCategory(cat).c_str(), code.c_str(), information.c_str());
        }
    }

    if ((numbytes = sendto(sockfd, res.c_str(), res.length(), 0, (struct sockaddr *)&their_addr, addr_len)) == -1)
    {
        perror("ServerCS:\n sendto");
        return;
    }
    cout << "The ServerCS finished sending the response to the Main Server." << endl;
}