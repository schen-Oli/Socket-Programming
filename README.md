
#### Name: Shuo Chen
#### ID: 5749775682
___
## **What I have completed in the assignment**
- Phase 0: Completed
- Phase 1: Completed
- Phase 2: Completed
- Phase 3: Completed
- Phase 4: Completed
- Extra Credits: Completed
   
## **What your code files are and what each one of them does**
   - __client.h, client.cpp:__ 
      1. Ask the user for a username and password.
      2. Send authentication request to serverM.
      3. Parse the authentication response, terminate program if authentication fails three times.
      4. Ask the user for courses and information they wanted to request.
      5. Format and send requests to serverM.
      6. Receive and format response display in console.

   - __serverC.h, serverC.cpp:__
      1. Read "cred.txt" file and construct a `<username, password>` map in memory.
      2. Receive authentication request from serverM.
      3. Check the map for the username and the corresponding password.
      4. Send authentication result back to serverM.

   - __serverCS.h, serverCS.cpp:__
      1. Read "cs.txt" file and construct a `<couseCode, courseInfo>` map in memory.
      2. Receive course information request from serverM.
      3. Check the map, retrieve information and format responses.
      4. Send the response to serverM.
   
   - __serverEE.h, serverEE.cpp:__
      1. Read "ee.txt" file and construct a `<couseCode, courseInfo>` map in memory.
      2. Receive course information request from serverM.
      3. Check the map, retrieve information and format response.
      4. Send the response to serverM.
   
   - __serverM.h, serverM.cpp:__
      1. Receive authentication request from Client.
      2. Encrypt username and password, and send reformatted message to serverC.
      3. Receive the authentication message from serverC and send back to Client.
      4. Receive request from Client, decide which server and what kind of information the Client need.
      5. Send reformatted request to ServerEE or ServerCS.
      6. Receive and reformat responses from ServerEE or ServerCS.
      7. Send the responses back to the Client. 

## **The format of all the messages exchanged:**
__1. Client -> ServerM:__
   - Authentication: `<Flag><Username>,<Password>` (eg. 1harry,vhjwefh)
      + Flag:
         * 0: Inform ServerM that Client wants to close connection
         * 1: An actual authentication request
   - Request: 
      - One course: `1<Coursecode>,<Type>` (eg. 1EE450,2)
         + Type:
            * 1: Credit
            * 2: Professor
            * 3: Days
            * 4: CourseName
      - Multiple courses: `2<Coursecode>` (eg. 2EE450)

__2. ServerM -> Client:__
   - Authentication: `<Flag>`)
      + 0: Success
      + 1: No Such User
      + 2: Password Not Match
   - Response `<Flag><Message>` (eg. 2Thu,Fri)
      + 1: Server failed to find course
      + 2: Server successfully retrieves the information
   
__3. ServerM -> ServerC:__
   - `<Username,Password>` in encrypted characters. (eg. fuds1?,sdga4qw)

__4. ServerC -> ServerM:__
   - `<Flag>`
      + 0: Success
      + 1: No Such User
      + 2: Password Not Match

__5. ServerM -> ServerCS / ServerEE:__
   - `<Coursecode>,<Flag>`
      + Flag:
         * 1: Credit
         * 2: Professor
         * 3: Days
         * 4: Course Name
         * 5: All (used to retrieve all information)

__6. ServerCS / ServerEE -> ServerM:__
   - `<Flag><Message>`
      + Flag:
         * 1: Course Not Found
         * 2: Successfully find the corresponding message

## **Any idiosyncrasy of your project** 
- No

## **Reused Code** 
- Did you use code from anywhere for your project? Yes
- If so, what functions and where they're from:
   - client.h:
      - `getPortNumber()` Refer the code from the assignment description
      - `getSolidSocketFd()` Refer the code from "Beej's Guide to Network Programming"