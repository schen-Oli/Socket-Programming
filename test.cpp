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
    string test = " EE450 HARYSD ";
    int start = 0;
    int end = test.length() - 1;
    while(test[start] == ' '){
        start++;
    }
    while(test[end] == ' '){
        end--;
    }
    test= test.substr(start, end);
    cout << "'" << test << "'" << end;

}

int main()
{
    trim();
}
