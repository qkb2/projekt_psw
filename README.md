We strongly encourage to compile the program from source using the provided makefile. Programs tested for Debian-like (kernel 5.15 generic, Linux Mint 21 works fine) and for OpenSuse Leap 15.2 (should work after recompilation using provided makefile). Standard compilation using -std=c99 standard and GNU GCC compiler.

In order to use the program correctly you first have to setup the server and run its' binary in a folder which provides config.txt with correct syntax. Example file provided. There should NOT be more than one server running at the same time. There can be up to 15 clients running at the same time (possible to change this value in .h file). Clients can use commands provided in console (auto-walkthrough) to perform various actions. Clients can be safely closed down with CTRL+C. Server should NOT be closed as long as there are any clients left (shutdown safety not yet implemented). Server can be safely shut down after disconnecting all clients using CTRL+C.

scanf safety is yet to be implemented fully.

Created by qkb2 (inf151825) and wylupek (inf151823). 