#include "serverCS.h"

#define SERVERC_PORT "21682" 
#define IP "127.0.0.1"

#define BACKLOG 10   // how many pending connections queue will hold

int main(void)
{
    // read file
    auto db = readFile("cs.txt");
    for(auto i = db.cbegin(); i != db.cend(); i++){
        string code = i->first;
        Course course = i->second;
        cout << code << (string)":" << endl;
        cout << course.credit << endl;
        cout << course.professor << endl;
        cout << course.days << endl;
        cout << course.name << endl;
    }

    struct addrinfo hints;
    struct addrinfo *servinfo, *i;
    int socketFD;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int status = getaddrinfo(IP, SERVERC_PORT, &hints, &servinfo);
    if(status != 0){
        fprintf(stderr, "getaddrinfo error: %d\n", status);
        exit(1);
    }

    for(i = servinfo; i != NULL; i = i->ai_next){
        socketFD = socket(i->ai_family, i->ai_socktype, i->ai_protocol);

        if(socketFD == -1){
            continue;
        }

        int bindRet = bind(socketFD, i->ai_addr, i->ai_addrlen);
        if(bindRet == -1){
            close(socketFD);
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (i == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    struct sockaddr_in sa;
    socklen_t sa_len = sizeof(sa);
    if (getsockname(socketFD, (struct sockaddr *)&sa, &sa_len) == -1) {
      perror("getsockname() failed");
      return -1;
   }

    printf("The ServerC is up and running using UDP on port %d\n", ntohs(sa.sin_port));
    close(socketFD);

    return 0;
}