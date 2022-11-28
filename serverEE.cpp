#include "serverEE.h"

int main(void)
{
    ReadFile();
    creatUDPConnection();
    while (1)
    {
        checkMessage();
    }
    return 0;
}