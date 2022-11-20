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

struct Course{
    string credit;
    string professor;
    string days;
    string name;
};

map<string, Course> readFile(string fileName){
    map<string, Course> ret;
    ifstream file;
    file.open(fileName);
    
    if(!file.is_open()){
        perror("failed to open credential file");
        exit(-1);
    }

    string line;
    while(getline(file, line)) {
        stringstream ss(line);
        string code, credit, professor, days, name;
        getline(ss, code, ',');
        getline(ss, credit, ',');
        getline(ss, professor, ',');
        getline(ss, days, ',');
        getline(ss, name, ',');
        Course course = {credit, professor, days, name};
        ret.insert(make_pair(code, course));
    }

    return ret;
}