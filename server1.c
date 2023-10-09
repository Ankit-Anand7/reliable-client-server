/*  
    COEN 233 Programming Assignment #1
    Name : Ankit Anand
    Student ID : W1652456
*/


// Libraries to be included
#include <stdio.h>      //Standard i/o functions
#include <stdlib.h>     //For standard library functions like atoi,exit,bzero,etc.
#include <string.h>     //For various string functions
#include <sys/types.h>  //For inclusion of other headers
#include <sys/socket.h> //For socket related functions and datatypes
#include <netinet/in.h> //For structures and functions for working with internet addresses
#include <arpa/inet.h>  //For manipulating internet addresses in a specific format
#include <sys/time.h>   //For time stamps 
#include <unistd.h>     //For sleep() method


/*

Packet header, packet trailer, packet ack and packet reject are declared in both client and server to enable the to correctly interpret the recived byte streams and print it

*/

// Declaration of the required data structures

//Data structure to store header
struct Packet_Header
{
    unsigned short int start_packetID;
    unsigned char client_ID;
    unsigned short int data;
    unsigned char seg_No;
    unsigned char length;
} header = {0xFFFF, 0xFF, 0xFFF1, 0x0, 0xFF};

//Data structure to store trailer
struct Packet_Trailer
{
    unsigned short int end_ID;
} trailer = {0xFFFF};

//Data structure to store ACK
struct packet_ACK
{
    unsigned short int start_packetID;
    unsigned char client_ID;
    unsigned short int ack;
    unsigned char seg_No;
    unsigned short int end_ID;
} acknowledge = {0xFFFF, 0xFF, 0xFFF2, 0x0, 0xFFFF};

//Data structure to store Reject
struct packet_Reject
{
    unsigned short int start_packetID;
    unsigned char client_ID;
    unsigned short int reject;
    unsigned short int rejectCode;
    unsigned char seg_No;
    unsigned short int end_ID;
} reject = {0xFFFF, 0xFF, 0xFFF3, 0xFFF4, 0x0, 0xFFFF};


// To print received packet details
void print_packet()
{
    printf( "\n-----Packet Number %d-----\n" ,header.seg_No);
    printf("Start of Packet ID \t: \"%#X\"\n", header.start_packetID);
    printf("Client ID \t\t: \"%#X\"\n", header.client_ID);
    printf("DATA \t\t\t: \"%#X\"\n", header.data);
    printf("Segment Number \t\t: \"%d\"\n", header.seg_No);
    printf("Length of payload \t: \"%#X\"\n", header.length);
    printf("End of Packet ID \t: \"%#X\"\n", trailer.end_ID);
    printf( "-------------------------\n\n" );
}

// To print ACK
void print_ack()
{
    printf( "\n----ACK from Server %d----\n" , acknowledge.seg_No);
    printf("Start of Packet ID \t: \"%#X\"\n", acknowledge.start_packetID);
    printf("Client ID \t\t: \"%#X\"\n", acknowledge.client_ID);
    printf("ACK \t\t\t: \"%#X\"\n", acknowledge.ack);
    printf("Received Segment Number : \"%d\"\n", acknowledge.seg_No);
    printf("End of Packet ID \t: \"%#X\"\n", acknowledge.end_ID);
    printf( "-------------------------\n\n" );
}

// To print rejected packets
void print_rejected()
{
    printf( "\n-----Reject Packet %d-----\n" ,reject.seg_No);
    printf("Start of Packet ID \t: \"%#X\"\n", reject.start_packetID);
    printf("Client ID \t\t: \"%#X\"\n", reject.client_ID);
    printf("REJECT \t\t\t: \"%#X\"\n", reject.reject);
    printf("Reject sub code \t: \"%#X\"\n", reject.rejectCode);
    printf("Received Segment Number : \"%d\"\n", reject.seg_No);
    printf("End of Packet ID \t: \"%#X\"\n", reject.end_ID);
    printf( "--------------------------\n\n" );
}

// To generate packets to be sent to client which are rejected due to some error
void reject_builder(unsigned short int code)
{
    reject.start_packetID = 0xFFFF;
    reject.client_ID = header.client_ID;
    reject.reject = 0xFFF3;
    reject.rejectCode = code;
    reject.seg_No = header.seg_No;
    reject.end_ID = 0xFFFF;
}

// Function to generate errors,if any
void error(char *msg)
{
    perror(msg);
    exit(1);
}

char buffer_payload[255];   //to store payload received from client
char packet[1024];          //to store the entire packet received from client
char server_reply[16];      //to store reply from server ie, ack/reject 

int main(int argc, char *argv[])
{
    //sleep(30);                    //To demonstrate ack timer
    int sock;                       //File descriptor of the socket
    int length;                     //Length of the server's address structure
    int n;                          //Number of bytes received from the client
    int count = 0;                  //counter for tracking the sequence number of received packets
    struct sockaddr_in server;      //Server socket address
    struct sockaddr_in frm_client;  //Client socket address
    socklen_t frm_client_len;       //Length of socket address

    //To check CLA, will generate error if CLA is not as required by Program
    //Here the arguments will be ./"xyz" and port number on which the server will LISTEN

    if (argc != 2)
    {
        fprintf(stderr, "Type: ./'xyz' port_number to deploy server\n");
        exit(1);
    }


    //Socket creation
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        error("Error in creating socket");
    }

    length = sizeof(server);                    //Stores the length of server socket address
    memset(&server, 0,length);                  //Sets all the bytes in server to zero till length "length"
    server.sin_family = AF_INET;                //Setting address family to IPV4
    server.sin_addr.s_addr = INADDR_ANY;        //Setting server to listen from all network interfaces
    server.sin_port = htons(atoi(argv[1]));     //To convert host port number to big-endian network byte order

    // Binds the socket to port number given by user in CLA and generates error, if any
    if (bind(sock, (struct sockaddr *)&server, length) < 0)
    {
        error("Error in binding socket");
    }
    frm_client_len = sizeof(struct sockaddr_in);


    //Deploying server to listen to client
    printf( "Server deployed and running without any errors\n" );       //Prints status of deployment of server
    while (1)                                                           //Keeping condition always true, and will generate errors through constant code to simulate assigned errors
    {
        n = recvfrom(sock, packet, 1024, 0, (struct sockaddr *)&frm_client, &frm_client_len);
        if (n < 0)
        {
            error("Error occured in receiving");
        }
        //Copies the header from the received packet from "packet" buffer
        memcpy(&header, packet, sizeof(header));

        //Copies payload from 
        memcpy(&buffer_payload, packet + sizeof(header), n - sizeof(header) - 2);
        buffer_payload[strlen(buffer_payload)] = '\0';

        //Copies the trailer from the received packet from "packet" buffer
        memcpy(&trailer, packet + n - 2, 2);

        //To display received packet information
        printf("\n\nReceived packet from client: \n");
        printf("Packet number: %d \n", header.seg_No);
        printf("Packet size: %d\n", n);
        printf("Content: %s", buffer_payload);
        print_packet();
        printf("\n\n");

        //Packet analysis and simulation of errors as instructed
        if (header.seg_No == count)
        {
            //Duplicate packet error
           
            printf( "Server: Packet Rejected.  \nError: Duplicate Packet\n");
            reject_builder(0xFFF7);                             //Builds reject message from byte stream received to print packet details
            print_rejected();                                   //Prints rejected packet details
            memset(server_reply, 0, 16);                        //Fills server_reply byte with zeroes to store reject message    
            memcpy(server_reply, &reject, sizeof(reject));      //Copies reject message 
            n = sendto(sock, &server_reply, sizeof(reject), 0, (struct sockaddr *)&frm_client, frm_client_len); //Sends to client
            if (n < 0)
            {
                error( "Server: Failed to send an ack.\n" );
            }
        }

        else if (header.seg_No != count + 1)
        {
            //Packet not in sequence error

            printf( "Server: Packet Rejected. \nError: Packet out of sequence\n" ); 
            reject_builder(0xFFF4);                            //Builds reject message from byte stream received to print packet details
            print_rejected();                                  //Prints rejected packet details
            memset(server_reply, 0, 16);                       //Fills server_reply byte with zeroes to store reject message   
            memcpy(server_reply, &reject, sizeof(reject));     //Copies reject message 
            n = sendto(sock, &server_reply, sizeof(reject), 0, (struct sockaddr *)&frm_client, frm_client_len); //Sends to client
            if (n < 0)
            {
                error( "Server: Failed to send an ack.\n" );
            }
            
        }
        else if (header.length != strlen(buffer_payload))
        {
            //Length mismatch in packet error
           
            printf( "Server: Packet Rejected.  \nError: Length mismatch\n" );
            reject_builder(0xFFF5);                             //Builds reject message from byte stream received to print packet details
            print_rejected();                                   //Prints rejected packet details
            memset(server_reply, 0, 16);                        //Fills server_reply byte with zeroes to store reject message 
            memcpy(server_reply, &reject, sizeof(reject));      //Copies reject message 
            n = sendto(sock, &server_reply, sizeof(reject), 0, (struct sockaddr *)&frm_client, frm_client_len); //Sends to client
            if (n < 0)
            {
                error( "Server: Failed to send an ack.\n" );
            }
            
        }
        else if (trailer.end_ID != 0xFFFF)
        {
            //missing end of packet error
        
            printf( "Server: Packet Rejected. \nError: End of packet missing\n");
            reject_builder(0xFFF6);                             //Builds reject message from byte stream received to print packet details
            print_rejected();                                   //Prints rejected packet details
            memset(server_reply, 0, 16);                        //Fills server_reply byte with zeroes to store reject message 
            memcpy(server_reply, &reject, sizeof(reject));      //Copies reject message 
            n = sendto(sock,&server_reply,sizeof(reject),0,(struct sockaddr*)&frm_client,frm_client_len);   //Sends to client
            if (n < 0){
                error( "Server: Failed to send an ack.\n" );
            }
        
        }
        else
        {

            //For acccepted packets
            printf( "Server: Packet accepted. Sending an ack.\n" );
            count++;                                                        //Increments count to run code for 5 packets
            acknowledge.start_packetID = 0xFFFF;                            //Start of packet as described in assignment
            acknowledge.client_ID = header.client_ID;                       //As received from client
            acknowledge.ack = 0xFFF2;                                       //ACK ID as described in assignment
            acknowledge.seg_No = header.seg_No;                             //The sequence no of received packet
            acknowledge.end_ID = 0xFFFF;                                    //End of packet as described in assignment
            printf("Size of ACK: %lu Bytes\n", sizeof(acknowledge));
            print_ack();                                                    //To print ACK in described format
            memset(server_reply, 0, 16);                                    //Fills server_reply byte with zeroes to store ACK message
            memcpy(server_reply, &acknowledge, sizeof(acknowledge));        //Copies ACK message in server_reply
            n = sendto(sock, &server_reply, sizeof(acknowledge), 0, (struct sockaddr *)&frm_client, frm_client_len);    //Sends to client
            if (n < 0)
            {
                error( "Server: Failed to send an ack.\n" );
            }
            //Reset the server count after receiving 5 packets successfully to test error messsages
            if (count == 5)
            {
                printf("\n\n\n\nResetting the counter to test other cases\n\n\n\n");
                count = 0;
            }
        }
    }
}