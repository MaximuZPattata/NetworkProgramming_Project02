**This project was done in a group of two(1. Max Sherwin and 2. Keerthivasan Kalaivanan)**

-------------------------------------------------------------------------<<Chat Module With Authenticator>>----------------------------------------------------------------------------------------------------

- In the previous project, we made a simple chat program using multiple socket connections from the server to the various clients connected to the server. The clients can allocate, join and leave a room. Other clients can join the same room and chat with others in the room. 
- In this project, the clients can now register to the network using email and password. The server receives the email and password and sends it to the authentication server using protocol buffers wrapped with length prefix whereas the messages sent from server to client or vice versa implement Big Endianness in their protocol when implementing serializing and deserializing in Buffer with the length of the message added to the start of the message.
- The Authentication server hashes the password with salt and stores it in the SQL database. 
- The Authentication server also checks the default constraints for email and password - The constraints being 1. Email should have '[A-Z/ a-z/ 0-9]' followed by '@' followed by '[A-Z/ a-z]' followed by '.'  in their email id, 2. Password should be more than 8 characters long.
- Once registration is completed, clients can login to the network using the same email and password. The server will check with the Authentication server for authentication of the email and password.
- The Authentication server will hash the login passwordwith the same salt used while regitering and compare it with the hashed password in the SQL and send the success or failure response using protocol buffers back to the chat server.
- The project makes use of the CLI User Interface in the best way possible.

-----------------------------------------------------------------------------<<How to build and use it>>-------------------------------------------------------------------------------------------------------

- Use the SQL file found in the path "ChatModuleWithAuthenticator/SQLFiles/create_schema_and_tables.sql" to create schema and tables in MySQL database.
- Open the solution found in the path "ChatModuleWithAuthenticator/ChatModuleDemo.sln". Once opened, build all 3 projects and once succeeded(probably will!), the .exe files on the path "ChatModuleWithAuthenticator/x64/Debug/TCPAuthenticationServer.exe", "ChatModuleWithAuthenticator/x64/Debug/TCPServer.exe" and "ChatModuleWithAuthenticator/x64/Debug/TCPServer.exe" can be opened to run the program, execute the exe files in this particular order :
	1. TCPAuthenticationServer
	2. TCPServer
	3. TCPClient(Execute N number of times, N = number of clients trying to connect to the server) 
- Once the terminals open, the socket connections and the success on connecting is displayed and the instructions as to how to operate the user interface for the client is also shown.
- Providing the same instructions below :
	- At first the client asks for the user's name
	- After that, User can press "Escape"/'ESC' to see the options available.
	- Press 'R' to register to the network. Enter Email and Password. 
	- Server responds with either success or failure. If failure, try the same steps again. If success, click Esc again. 
	- If succeeded in registering, Press 'ESC' and 'L' to login using the same email and password.
	- If succeeded, the user can join a room now.
	- Press 'J' to join a room or Press 'O' to logout from the network or press 'ESC' again to cancel option loadout.
	- Once 'J' is pressed, the interface prompts for the room name.
	- After typing the room name, chat is enabled for the room. Other users can join the same room by entering the same room name when prompted.
	- By pressing 'TAB', the user will be able to chat with other users in the room. 
	- Else, user can press 'ESC' for more options. 
	- Press 'E' to exit the room or press 'ESC' again to cancel option loadout.
	- Once exited the room, user can join again to the same room or a different one by pressing 'ESC' and 'J' keys.

- Also in the "TCPAuthenticationServer project->authMain.cpp", the user can change the database's username and password to connect to the user's database. By default the username is "root" and password is "root".

-------------------------------------------------------------------------<<Limitations of the project>>--------------------------------------------------------------------------------------------------------

- Closing a Client terminal terminates the whole program's functionality. We tried for a workaround, but terminating the whole program(including server and other clients) was the only solution we could come up it as we pushed this task to the very last. 

- Github isnt allowing to push certain files due to the file size constraint. So Im not able to commit after certain changes. But I am providing the github link anyway :
	-"https://github.com/MaximuZPattata/SimpleChatProgram.git"