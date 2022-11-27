
### Name: Shuo Chen
### ID: 5749775682
---
## **What I have done in the assignment:**
1. Phase 0...
   
---
## **What your code files are and what each one of them does.**

client.h
client.cpp
serverC.h
serverC.cpp
serverM.h
serverM.cpp
serverCS.h
serverCS.cpp
serverEE.h
serverEE.cpp

---
## **The format of all the messages exchanged:**
__1. client -> serverM:__
   - Authentification: `<Flag><Username>,<Password>` (eg. 1harry,vhjwefh)
      + Flag = 0: Inform serverM that client wants to close connection
      + Flag = 1: An actual authentification request
   - Request: 
      + One course: `1<Coursecode>,<Type>` (eg. 1EE450,2)
         * Type = 1: Credit
         * Type = 2: Professor
         * Type = 3: Days
         * Type = 4: CourseName
      + Multiple courses: `2<Coursecode> <Coursecode> <Coursecode>` (eg. 2EE450 CS561 DS304)


__2. serverM -> client:__
   - Authentification: `<Authentification>` (eg. 0)
      + 0: Success
      + 1: No Such User
      + 2: Password Not Match
   - Response `<Flag><Response>` (eg. 2Thu,Fri)
      + Flag = 0: Server finished sending all messages
      + Flag = 1: Server failed to find course
      + Flag = 2: Server successfully retrieves the information
   
__3. serverM -> serverC:__
   - `<Username,Password>` in encrypted characters. (eg. fuds1-?,fhja4-?)

__4. serverC -> serverM:__
   - `<Authentification>` (eg. 0)
      + 0: Success
      + 1: No Such User
      + 2: Password Not Match

__5. serverM -> serverCS:__

g. Any idiosyncrasy of your project. It should say under what conditions the project
fails, if any.

h. Reused Code: Did you use code from anywhere for your project? If not, say so. If so,
say what functions and where they're from. (Also identify this with a comment in the source code.)
