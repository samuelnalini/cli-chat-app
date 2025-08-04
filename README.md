# CLI Chat App
*A simple command line chat application*

## ABOUT:

CLI Chat App is a simple command line application that allows communication between multiple users. It uses a client-server architecture, allowing for scalability and ease of use. Being written in C++, this application is fast and lightweight, making use of low level concepts such as sockets, while also using threads for asynchronous operations.

<sub>*Please see [CONSIDERATIONS](#considerations) before attempting to use this!*</sub>

# HELP SECTION:

The project is split up into two separate executables. The server listens to instructions and forwards messages to the clients connected to it.
The client on the other hand connects to the server and is what users interact with. They are able to input messages which get sent to the server.

## DEPENDENCIES

There are some requirements to run this project.

```
sudo apt update
sudo apt install build-essential cmake curl git unzip pkg-config autoconf automake libtool zip libncurses-dev libsodium-dev
```

## BUILD INSTRUCTIONS

Building the project is a simple process.

First, clone the repo:

```
git clone --recurse-submodules -j8 https://github.com/samuelnalini/cli-chat-app.git
```

Create a build directory

```
cd cli-chat-app && mkdir -p build && cd build
```

Run:

```
cmake -G "Unix Makefiles" ..
```

```
make
```

*If you have any problems with this part, see the [DEPENDENCIES](#dependencies) section.*


### Specifying IP and Port

**Server**

By default the server will listen on `port 8080`

You can change this when starting the server by specifying `./server <port>`

**Client**

The client will connect to localhost by default. `127.0.0.1 on port 8080`

You can change this when starting the client by specifying `./client <ip> <port>`

*Connections to outside servers are possible, but please see [CONSIDERATIONS](#considerations)!*


## SERVER INSTRUCTIONS

You can start the server simply by running it as an executable:

(chat-app/build) `./server`

or

(chat-app/build) `./server <ip> <port>`

It will then initialize and begin listening on the specified port.

## CLIENT INSTRUCTIONS

The client is very similar to the server, but must specify an IP and port in order to communicate with a server.

### THE SERVER MUST BE RUNNING IN ORDER FOR THE CLIENT TO CONNECT

You can run the client as an executable:

(chat-app/build) `./client`

or

(chat-app/build) `./client <ip> <port>`

It will prompt the user to pick a username and then join the chat.

## CLIENT COMMANDS

The client is able to send special commands to the server by prefixing them with `/`

As of right now, there is only the `/exit` command, which will exit the server.

Not very exciting, but more commands will be added in the future.

### CONSIDERATIONS:

*NOTE: As of right now, messages are **__NOT__ ENCRYPTED!** This means that this app is **__not__ secure** and others may be able to read chat logs. Do not use this for any real work. You have been warned.*

*NOTE: This was designed to run on Linux machines! The socket code for other platforms are slightly different and will not work without modification!*

*This application is receiving active updates. It will improve in the future.*

### THINGS I'VE LEARNED THROUGH THIS PROJECT:
  - User input
  - CMake Build Tools
  - Package management through vcpkg
  - ~~Message encryption through libsodium~~
  - Networking through sockets and packets
  - TCP Socket Implementation
  - Basic Client/Server Architecture
  - The use of threads, multithreading and thread safety
  - epoll() and event-based systems.
  - Debugging skills
  - Development in Linux
