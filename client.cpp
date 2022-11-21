#include "client.h"

int main(void)
{
    cout << "The client is up and running" << endl;
    
    //set up socket 
    int sockfd;
    sockfd = getSolidSocketFd();
    
    //authentification
    auth(sockfd);

close(sockfd);
    return 0;
}