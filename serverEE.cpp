#include "serverEE.h"

int main(void)
{
    ReadFile("ee.txt");
    creatUDPConnection();
    while (1)
    {
        checkMessage();
    }
    return 0;
}