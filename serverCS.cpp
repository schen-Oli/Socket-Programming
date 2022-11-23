#include "serverCS.h"

int main(void)
{
    readFile("cs.txt");
    creatUDPConnection();
    while (1)
    {
        checkMessage();
    }
    return 0;
}