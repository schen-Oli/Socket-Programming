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
#include <set>
#include <vector>

#define IP "127.0.0.1"
#define PORT_SERVER_M "25682" // port number of serverM
#define MAXDATASIZE 100

#define CREDIT '1'
#define PROFESSOR '2'
#define DAYS '3'
#define COURSE_NAME '4'
#define FULL '5'

using namespace std;

string username;

// Get the port number given socket fd
uint16_t getPortNumber(int sockfd)
{
    struct sockaddr_in sa;
    socklen_t sa_len = sizeof(sa);

    // refer the code in assignment description 
    if (getsockname(sockfd, (struct sockaddr *)&sa, &sa_len) == -1)
    {
        perror("Client:\n getsockname() failed");
        return 0;
    }

    return ntohs(sa.sin_port);
}

// Trim the leading and tailing spaces from a string
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

// Give the category in string format and return its code
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
    else if (category == "CourseName")
    {
        return COURSE_NAME;
    }
    return '0';
}

// Get a valid socket fd and connect to serverM
// Code in this function refers to "Beej's Guide to Network Programming"
int getSolidSocketFd()
{
    struct addrinfo hints, *servinfo, *p;
    int sockfd;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int rv;
    if ((rv = getaddrinfo(IP, PORT_SERVER_M, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("client: socket");
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
        fprintf(stderr, "Client: failed to connect\n");
        exit(1);
    }

    freeaddrinfo(servinfo);
    return sockfd;
}

// Ask the user for username and password
// terminate program if authentication fails 3 times
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

        // add 1 at the front of request, tell SeverM that Client wants to keep connected
        string data = "1" + username + "," + password;

        if (send(sockfd, data.c_str(), data.size(), 0) != data.size())
        {
            perror("Authentication - send");
            exit(1);
        }

        cout << username << " sent an authentication request to the main server." << endl;

        char buf[MAXDATASIZE];
        int numbytes;
        if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
        {
            perror("Authentication - recv");
            exit(1);
        }

        buf[numbytes] = '\0';

        if (buf[0] == '1')
        {
            printf("%s received the result of authentication using TCP over port %d. Authentication failed: Username Does not exist\n", username.c_str(), getPortNumber(sockfd));
            printf("Attempts remaining:%d\n", cnt);
        }
        else if (buf[0] == '2')
        {
            printf("%s received the result of authentication using TCP over port %d. Authentication failed: Password does not match\n", username.c_str(), getPortNumber(sockfd));
            printf("Attempts remaining:%d\n", cnt);
        }
        else
        {
            printf("%s received the result of authentication using TCP over port %d. Authentication is successful\n", username.c_str(), getPortNumber(sockfd));
            return;
        }
    }

    // Send 0 to tell the serverM that client wants to end connection
    if (send(sockfd, "0", 1, 0) == -1)
    {
        perror("Authentication - send");
        exit(1);
    }

    close(sockfd);
    cout << "Authentication Failed for 3 attempts. Client will shut down." << endl;
    exit(1);
}

// User only request for one course
// req: requst that will be send to the corresponding server
// code: the code that user entered
void requestOneCourse(string req, string code, int sockfd)
{
    string category;
    //loop until user enter the correct format of category
    while (getCat(category) == '0')
    {
        if (!category.empty())
        {
            cout << "Wrong category format, please choose from (Credit / Professor / Days / CourseName)" << endl;
        }
        category.clear();
        cout << "Please enter the category (Credit / Professor / Days / CourseName): ";
        cin >> category;
    }

    req += code + ",";
    req += getCat(category);

    // send the request to serverM
    if (send(sockfd, req.c_str(), req.size(), 0) == -1)
    {
        perror("requestOneCourse - send");
        exit(1);
    }

    cout << username << " sent a request to the main server." << endl;

    char buf[MAXDATASIZE];
    memset(buf, 0, sizeof buf);
    int numbytes;
    // recieve result from ServerM
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
    {
        perror("requestOneCourse - recv");
        exit(1);
    }
    else
    {
        printf("The client received the response from the Main server using TCP over port %d.\n", getPortNumber(sockfd));
    }

    buf[numbytes] = '\0';

    if (buf[0] == '1')
    {
        printf("Didn’t find the course: %s.\n", code.c_str());
    }
    else
    {
        string resposne = string(buf + 1, numbytes - 1);
        printf("The %s of %s is %s.\n", category.c_str(), code.c_str(), buf + 1);
    }
}

// User wants to request information for multiple courses
// req: requst that will be send to the corresponding server
// code: the code that user entered
void requestMultiCourse(string req, string code, int sockfd)
{
    set<string> codeSet; // hashset is used to remove duplicate inputs
    vector<string> codes;

    string tmpCode;
    int left = 0, right = 0;

    while (right < code.length())
    {
        if (code[right] == ' ')
        {
            tmpCode = code.substr(left, right - left);
            codeSet.insert(tmpCode);
            codes.push_back(tmpCode);
            while (right < code.length() && code[right] == ' ')
            {
                right++;
            }
            left = right;
        }

        right++;
    }
    tmpCode = code.substr(left, right - left);
    codeSet.insert(tmpCode);
    codes.push_back(tmpCode);

    cout << username << " sent a request with multiple CourseCode to the main server." << endl;
    bool printed = false;
    for (int i = 0; i < codes.size(); i++)
    {
        string curr = codes[i];
        if (codeSet.count(curr) == 0)
        {
            continue;
        }else{
            codeSet.erase(curr);
        }

        int numbytes;
        string reqMessage = req + curr;
        // send multi course request to serverM
        if (send(sockfd, reqMessage.c_str(), reqMessage.size(), 0) == -1)
        {
            perror("requestMultiCourse - send");
            exit(1);
        }

        char buf[MAXDATASIZE];
        memset(buf, 0, sizeof buf);
        // recieve multi course response from serverM
        if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
        {
            perror("requestMultiCourse - recv");
            exit(1);
        }
        else if (!printed)
        {
            printf("The client received the response from the Main server using TCP over port %d.\n", getPortNumber(sockfd));
            printf("CourseCode: Credits, Professor, Days, Course Name\n");
            printed = true;
        }

        buf[numbytes] = '\0';

        string response = string(buf + 1, strlen(buf) - 1);
        cout << response << endl;
    }
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

    cout << "\n-----Start a new request-----" << endl;
}
