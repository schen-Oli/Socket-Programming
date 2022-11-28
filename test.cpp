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
#include <set>
#include <stdio.h>
#include <ctype.h>

#define MAXDATASIZE 100
using namespace std;

void trim()
{
    string code = "EE450 CS561 EE456 DS123 EE450";
    set<string> codes;

    string tmpCode;
    int left = 0, right = 0;
   
    while (right < code.length())
    {
        if (code[right] == ' ')
        {
            tmpCode = code.substr(left, right - left);
            codes.insert(tmpCode);
            while (right < code.length() && code[right] == ' ')
            {
                right++;
            }
            left = right;
        }

        right++;
    }
    tmpCode = code.substr(left, right - left);
    codes.insert(tmpCode);

    for (auto it = codes.begin(); it !=
                             codes.end(); ++it)
        cout << "'" << *it << "'" <<endl;
}

int main()
{
    trim();
}
