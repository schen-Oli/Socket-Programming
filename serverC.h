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

using namespace std;

map<string, string> readFile(string fileName){
    map<string, string> ret;
    ifstream file;
    file.open(fileName);
    
    if(!file.is_open()){
        perror("failed to open credential file");
        exit(-1);
    }

    string line;
    while(getline(file, line)) {
        stringstream ss(line);
        string username;
        string password;
        getline(ss, username, ',');
        getline(ss, password);
        ret.insert(make_pair(username, password));
    }

    return ret;
}