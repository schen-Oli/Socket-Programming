#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <iostream>

#include <stdio.h>
#include <ctype.h>

#include "client.h"

#define MAXDATASIZE 100
using namespace std;

void trim()
{
    string password = "abbbb";
    while (password[password.length() - 1] == 'b')
    {
        password.erase(password.length() - 1);
    }

    cout << password <<"'" << endl;
}

int main()
{
    trim();
}
