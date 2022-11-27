#include "serverM.h"
#define BACKLOG 10

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
            continue;
        }

        struct sockaddr_storage their_addr;
        socklen_t sin_size = sizeof their_addr;

        int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

        if (new_fd == -1)
        {
            perror("accept");
        }

        if (!fork())
        {
            close(sockfd); // child doesn't need the listener
            acceptConnections(new_fd);
        }

        close(new_fd); // parent doesn't need this
    }

    return 0;
}