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

void trim(){
    string req;
    string code = "EE450 CS451  EE454 DS123";
    int index = 0;
    while (index < code.length())
    {
        if (code[index] == ' ')
        {
            req.append(" ");
            while (index < code.length() && code[index] == ' ')
            {
                index++;
            }
        }

        if (isalpha(code[index]))
        {
            string dep;
            while (index < code.length() && isalpha(code[index]))
            {
                dep += code[index];
                index++;
            }
            req += dep;
        }

        if (isdigit(code[index]))
        {
            req += code[index];
            index++;
        }
    }
    cout << "'" << code << "'" << endl;
    cout << "'" << req << "'" << endl; 
}

int main()
{
    trim();
}
