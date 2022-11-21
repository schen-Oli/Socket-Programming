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

#define MAXDATASIZE 100
using namespace std;

void encryption(char *buf){
    for(int i = 0; i < strlen(buf); i++){
        char tmp = buf[i];
        if(isalpha(tmp)){
            int init;
            if(isupper(tmp)){
                init = 'A';
            }else{
                init = 'a';
            }

            buf[i] = (buf[i] - init + 4) % 26 + init;
        }else if(isdigit(tmp)){
            buf[i] = buf[i] - '0' + 4 >= 10 ? buf[i] + 4 - 10 : buf[i] + 4; 
        }
    }
}

int main()
{
    char buf[MAXDATASIZE] = "Welcome to EE450!";
    printf("%s\n", buf);
    encryp(buf);
    printf("%s\n", buf);

    char buf2[MAXDATASIZE] = "199@$";
    printf("%s\n", buf2);
    encryp(buf2);
    printf("%s\n", buf2);

    char buf3[MAXDATASIZE] = "0.27#&";
    printf("%s\n", buf3);
    encryp(buf3);
    printf("%s\n", buf3);

    return 0;
}
