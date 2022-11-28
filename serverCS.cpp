#include "serverCS.h"

int main(void)
{
    readFile();
    creatUDPConnection();
    while (1)
    {
        checkMessage();
    }
    return 0;
}