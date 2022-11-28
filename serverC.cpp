#include "serverC.h"

int main(void)
{
    readFile(); 
    creatUDPConnection(); 
    cout << "The ServerC is up and running using UDP on port " << PORT << "." << endl;

    while (1)
    {
        checkMessage();
    }

    close(sockfd);
    return 0;
}