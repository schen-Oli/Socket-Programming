#include "serverEE.h"

int main(void)
{
    readFile("ee.txt");
    creatUDPConnection();
    while (1)
    {
        checkMessage();
    }
    return 0;
}