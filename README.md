# CLI Chat App
*A simple command line chat application*

### ABOUT:

CLI Chat App is a simple command line application that allows communication between multiple users. It uses a client-server architecture, allowing for scalability and ease of use. Being written in C++, this application is fast and lightweight, making use of low level concepts such as sockets, while also using threads for asynchronous operations.

*Please see considerations before attempting to use this!*

### INSTRUCTIONS:

The project is split up into two separate executables. The server listens to instructions and forwards messages to the clients connected to it.
The client on the other hand connects to the server and is what users interact with. They are able to input messages which get sent to the server.

#### - SERVER INSTRUCTIONS

You can start the server simply by running it as an executable:

(chat-app/build) `./server`

or

(chat-app/build) `./server <ip> <port>`

It will then initialize and begin listening on the specified port.

#### - CLIENT INSTRUCTIONS

The client is very similar to the server. It runs on the same IP and port to be able to communicate with it.

You can run the client as an executable:

(chat-app/build) `./client`

or

(chat-app/build) `./client <ip> <port>`

It will prompt the user to pick a username and then join the chat.

#### Changing the default IP and port

By default, both the server and the client are set to run on `127.0.0.1 on port 8080`
This is useful for LAN connections, but not so much for public use. If you'd like to specify the IP or port you can do so by running the following:

`server 127.0.0.1 1234`
`client 127.0.0.1 1234`

*NOTE: If you specify the server IP or port you must do the same for the client, otherwise they won't be able to communicate.
Also, if you want to specify either the port or the IP you MUST specify both, even if they are the same as the default.*


### CONSIDERATIONS:

*NOTE: As of right now, messages are **NOT ENCRYPTED!** This means that this app is not secure and others may be able to read chat logs. Do not use this for any real work. You have been warned.*

*NOTE: This was designed to run on Linux machines! The socket code for other platforms are slightly different and will not work without modification!*

*This application is receiving active updates. It will improve in the future.*

### THINGS I'VE LEARNED THROUGH THIS PROJECT:
  - User input
  - CMake Build Tools
  - TCP Socket Implementation
  - Basic Client/Server Architecture
  - The use of threads, multithreading and thread safety
  - Debugging skills
  - Development in Linux
