#include "serverM.h"

int main(void)
{
    int sockfd = getTcpSocketFd();
    if(sockfd < 0){
        perror("sockfd");
        exit(1);
    }
    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }
    

    cout << "sockfd is " << sockfd << endl;
    
    while(1){
        acceptConnections(sockfd);
    }

    return 0;


}