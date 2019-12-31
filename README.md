## Logging Server
> This repository contains two applets, server and client.

## Features
- Userspace programs that can be run without root permissions
- The server can be accessed by multiple clients in parallel
- The client can connect to any IP/port on the internet where the server is hosted
- The server saves the logs to disk as soon as possible

## Specification
Please see [SPEC.md](spec.md) file to understand the problem that this project is trying to resolve.

## Implementation
The connection between the server and the client is realized using TCP sockets. The server uses POSIX threads to parallelize clients. One thread is issued for each client.

## Building and running
To run my project you need to compile from source. Do `make` in the root directory.

This will create two executables: __client__ and __server__

The client takes two arguments (remote IP / remote port)
The server takes three arguments (IP to bind to / port to bind to / filename to output logs in `/var/log`)

Not using any ports/IPs will default to running on localhost:5000

Once running, the client takes strings from STDIN and sends them to the server. The server will output the messages to the log file 

## Memory checking
- To check for memory leaks, errors and dangling pointers, I have used __valgrind__
 > Valgrind is an analysis tool that checks memory at run time
- You can find it on [github/tklengyel/valgrind](https://github.com/tklengyel/valgrind)

## License
This project is licensed under the Apache License Version 2.0 - see the [LICENSE.md](LICENSE.md) file for details

## Built with
- [Make](https://www.gnu.org/software/make/manual/make.html) - GNU Make
- [CLion](https://www.jetbrains.com/clion/) - IDEA Project
- [valgrind](https://valgrind.org/) - memory testing
