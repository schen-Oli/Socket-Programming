# **UNIX Socket Programming**
A USC web registration system. A student will use the client to access the central web registration main server, which will forward their requests to the department servers in each department. For each department, the department server will store the information of the courses offered in this department. Additionally, a credential server will be used to verify the identity of the student.

Demo: https://youtu.be/ZShD-chIF84
___
   
## Usage
Generate excutable files
```
make all
```
Start servers:
- serverM: Main server, communicate with client
- serverC: Credential server
- serverCS: Server of CS department
- serverEE: Server of EE department
```
./serverM
./serverC
./serverCS
./serverEE
```
Start client
```
./client
```

## License

[MIT](https://choosealicense.com/licenses/mit/)

For future EE450 Student at USC, this project is recorded in Turnitin, so please DO NOT copy any code from this file or it will result in academic misconduct.
The author of this project will not be responsible for any consequence.