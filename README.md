# TCP/IP Chat Server

This is a TCP/IP chat server implemented in C++. The server allows multiple clients to connect and communicate with each other through a chat room.

![TCP/IP Chat Server Chats](assets/chat.png)
![TCP/IP Chat Server Logs](assets/logs.png)

## Features

- Multiple clients can connect to the server and communicate in a chat room
- The server logs messages sent by clients to a text file
- The server supports basic commands for listing connected clients and disconnecting clients
- Proper error handling and memory management techniques are used

## Requirements

- g++ compiler (GCC or Clang recommended)

## Usage

To build the program, run the following command:

```bash
g++ -O2 -g -Wall -Werror -o chat_server ./src/client.cpp ./src/client_server.cpp ./src/server.cpp
```

To run the program, use the following command:

```bash
./chat_server [port]
```

Replace [port] with the port number you want the server to listen on. For example, to run the server on port 8080, use the following command:

```bash
./chat_server 8080
```

Clients can connect to the server using a Telnet client or similar program. The server logs messages sent by clients to a file called chat_log.txt in the same directory as the program.

## Commands

The server supports the following commands:

- `/list`: List all connected clients
- `/kick [client_id]`: Disconnect the client with the specified ID

To use a command, simply type it in the chat room and press Enter.

## Notes
- The code is well-organized and commented for readability.
- The server uses the select function to handle multiple clients simultaneously.
- Clients are identified by their socket file descriptors.
- The server uses a linked list to keep track of connected clients.

## Example

Here is an example of how to use the server:

Start the server on port 9000:
```bash
./chat_server 9000
```

Connect to the server using a Telnet client:

```bash
telnet localhost 9000
```

Send a message to the chat room:
```
Hello, everyone!
```

Use the /list command to see a list of connected clients:

```bash
/list
```

Output:

```
Connected clients:
5. 127.0.0.1:46242
6. 127.0.0.1:46254
7. 127.0.0.1:46268
8. 127.0.0.1:46280
```

Use the /kick command to disconnect a client:

```bash
/kick 5
```

Output:

```bash
Client 5 has been kicked.
```

## Authors

- Maximilian Edison
- Max Base

Copyright 2023, Max Base
