#include "serverDepUtil.h"

int main(void)
{
    setServerName("severCS");
    readFile("cs.txt");
    creatUDPConnection(PORT_CS);
    printf("The serverCS is up and running using UDP on port %s.\n", PORT_CS);

    while (1)
    {
        checkMessage();
    }
    return 0;
}