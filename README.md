# README

## Introduction and Features
This project focuses on inter-process communication, including implementation of a server and a client which communicate using a message queue. The program will create a number of client processes and they will each send the server some request using the message queue. The server will create several threads in order to handle the clients. When all the clients are done, the server threads will send the global result to corresponding clients.

## Setup
To execute our program, open two separate terminals: one serves as client while the other as server. To compile and starts our program, enter the following commands in the first terminal:
```
$ gcc -o server server.c
$ ./server number_of_threads
```
and the following commands in the second terminal:
```
$ gcc -o client client.c
$ ./client number_of_process path_to_textfiles
```
Press enter and you should be able to see the result.

## Assumptions
1. 'server' program will run before the 'client' program.
2. Both server and client will be executed on the same machine.
3. The files in the input directory of the client will have a single word per line not exceeding 50 characters and the number of words per file can vary.
4. The number of files present in the input directory to client should exceed the number of client processes unless it is an empty folder.
5. The argument for server program and the second argument for client program will be the
same (number of client processes).
