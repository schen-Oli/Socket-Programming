#include "serverDepUtil.h"

int main(void)
{
    setServerName("severEE");
    readFile("ee.txt");
    creatUDPConnection(PORT_EE);
    printf("The serverEE is up and running using UDP on port %s.\n", PORT_EE);

    while (1)
    {
        checkMessage();
    }

    return 0;
}