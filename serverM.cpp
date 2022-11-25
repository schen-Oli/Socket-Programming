#include "serverM.h"

int main(void)
{
    cout << "The main server is up and running." << endl;

    int sockfd = getTcpSocketFd();
    if (sockfd < 0)
    {
        perror("sockfd");
        exit(1);
    }

    while (1)
    {
        if (listen(sockfd, BACKLOG) == -1)
        {
            perror("listen");
            exit(1);
        }

        acceptConnections(sockfd);
    }

    return 0;
}