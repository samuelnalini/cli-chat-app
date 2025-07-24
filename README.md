# CLI Chat App Template
*A simple chat app that runs in the command line.*

*Please see considerations!*

## INSTRUCTIONS:

If you'd like to run this yourself, this project is broken up into two separate executables.

### Changing the default IP and port

The server is set to run on `127.0.0.1 on port 8080`
If you want to change this you can do so by modifying server/src/headers/server.hpp
*NOTE: If you change the server IP or port you must do the same for the client*
The client ip and port is found in client/src/headers/client.hpp

### SERVER INSTRUCTIONS

You can start the server simply by running it as an executable:

(chat-app/build) `./server`

It will then initialize and begin listening on the specified port.

### CLIENT INSTRUCTIONS

The client is very similar to the server. It runs on the same IP and port to be able to communicate with it.

You can run the client as an executable:

(chat-app/build) `./client`

It will prompt the user to pick a username and then join in the chat.

### CONSIDERATIONS:

*NOTE: Messages are **NOT ENCRYPTED!** This means that this app is **not secure** and others may be able to read chat logs. Do not use this for any real work.* (I will implement encryption in the future)

*NOTE: I am still learning C++! There are certainly better ways to go about this and I am using this project as a learning experience.*

*NOTE: This was designed to run on Linux machines! The socket code for other platforms are slightly different and will not work without modification!* (I will make this cross platform in the future*(tm)*)
