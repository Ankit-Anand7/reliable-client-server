# reliable-client-server
Reliable client-server communication using custom protocol on top of UDP to ensure reliable packet transfer,error handling, retransmission, packet ack and rejection mechanism.



Instructions:-
--------------

Program: Client using customized protocol on top of UDP protocol for sending information to the server(Communication between one client and one server).


Steps to compile and run the program:

  i. Open terminal and change directory using command cd to the folder in which the programs are stored.
 ii. Compile server program by using command gcc -o "xyz" server1.c
iii. Compile client program by using command gcc -o "xyz" client1.c
 iv. Run server program first by using command ./"xyz" port_number
  v. Run client program after that by using command ./"xyz" localhost port_number


Notes:

   i. Make sure gcc compiler is installed in your system.
  ii. If you are using clang, use clang commands instead of gcc.
 iii. Be very precise to run server program first and not the client program.
  iv. Server program expects 2 command-line arguments, ie, executable file name and port number.
   v. Client program expects 3 command-line arguments, ie, executable file name, host name and port number.
  vi. I am going to demonstrate on my system, so both client and server will be on the same system, so hostname will be localhost.
 vii. Data packet headers and trailers are intended to be used in both client and server and thus is defined in both files to avoid compilation errors.
viii. To test ack_timer, either uncomment the sleep() method in main function of server program or run both programs on different port numbers.
