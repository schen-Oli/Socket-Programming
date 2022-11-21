#include "serverC.h"

// Credential Server

int main(void)
{
    readFile("cred.txt"); // read file to a map

    creatUDPConnection(); // create listener

    cout << "The ServerC is up and running using UDP on port " << PORT << endl;

    while (1)
    {
        checkMessage();
    }

    freeaddrinfo(servinfo);
    close(sockfd);
    return 0;
}