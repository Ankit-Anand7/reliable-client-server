/*  
    COEN 233 Programming Assignment #1
    Name : Ankit Anand
    Student ID : W1652456
*/

//Libraries to be included
#include <stdio.h>          //Standard i/o functions
#include <stdlib.h>         //For standard library functions like atoi,exit,bzero,etc.
#include <string.h>         //For various string functions
#include <sys/types.h>      //For inclusion of other headers
#include <sys/socket.h>     //For socket related functions and datatypes
#include <netinet/in.h>     //For structures and functions for working with internet addresses
#include <netdb.h>          //For hostname and address resolutions
#include <arpa/inet.h>      //For manipulating internet addresses in a specific format
#include <sys/time.h>       //For time stamps and timers


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



// To print sent packet details
void print_packet()
{
    printf( "\n---- Packet Number %d ----\n" ,header.seg_No);
    printf("Start of Packet ID \t:\"%#X\"\n", header.start_packetID);
    printf("Client ID \t\t:\"%#X\"\n", header.client_ID);
    printf("DATA \t\t\t:\"%#X\"\n", header.data);
    printf("Segment Number \t\t:\"%d\"\n", header.seg_No);
    printf("Length of payload \t:\"%#X\"\n", header.length);
    printf("End of Packet ID \t:\"%#X\"\n", trailer.end_ID);
    printf( "-------------------------\n\n" );
}

// To print ACK
void print_ack()
{
    printf( "\n----ACK from Server %d----\n" , acknowledge.seg_No);
    printf("Start of Packet ID \t:\"%#X\"\n", acknowledge.start_packetID);
    printf("Client ID  \t\t:\"%#X\"\n", acknowledge.client_ID);
    printf("ACK \t\t\t:\"%#X\"\n", acknowledge.ack);
    printf("Received Segment Number Number :\"%d\"\n", acknowledge.seg_No);
    printf("End of Packet ID \t:\"%#X\"\n", acknowledge.end_ID);
    printf( "-------------------------\n\n" );
}

// To print rejected packets
void print_rejected()
{
    printf( "\n-----Reject Packet %d-----\n" ,reject.seg_No);
    printf("Start of Packet ID \t:\"%#X\"\n", reject.start_packetID);
    printf("Client ID  \t\t:\"%#X\"\n", reject.client_ID);
    printf("REJECT \t\t\t:\"%#X\"\n", reject.reject);
    printf("Reject sub code \t:\"%#X\"\n", reject.rejectCode);
    printf("Received Segment Number :\"%d\"\n", reject.seg_No);
    printf("End of Packet ID \t:\"%#X\"\n", reject.end_ID);
    printf( "-------------------------\n\n" );
}

// To generate packets rejected by server and received over network
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

char buffer_payload[255];           //to store payload received from client
char packet[1024];                  //to store the entire packet sent from client
char server_reply[16];              //to store reply from server ie, ack/reject 
int sock;                           //File descriptor of the socket
int n;                              //Number of bytes sent by the client
int packet_num;                     //PAcket number used to simulate cases as per instructions
int count;                          //counter to keep track of number of attempts client tries to send any packet to server
struct sockaddr_in server;          //Server socket address
struct sockaddr_in frm_client;      //Client socket address
struct hostent *host_info;          //Host info
socklen_t length;                   //Length of frm_client socket address structure
char payload[256];                  //Payload
struct timeval timeout;             //For ack timer


// To build packets which will be sent to server
void packet_builder(int seg, char *payload)
{
    header.seg_No = seg;                                                //Segment number
    header.length = strlen(payload);                                    //Length of payload             
    memset(packet, 0, 264);                                             //Sets all bytes in packet from length 0 to 264 as zeroes
    memcpy(packet, &header, sizeof(header));                            //Copies header in packet buffer
    memcpy(packet + sizeof(header), payload, strlen(payload));          //Copies payload in packet buffer
    memcpy(packet + sizeof(header) + strlen(payload), &trailer, 2);     //Copies trailer in packet buffer
    printf("Packet created:");                                          //Prints to stdout
    print_packet();                                                     //Prints the created packet and all its field
}

// Function to transmit packet to server
void send_packet()
{
    //To try transmitting any packet for maximum three times
    count = 3;
    while (count >= 0)
    {
        // To send packet, first attempt
        if (count == 3)
        {
            printf("\nSending Packet: %s\n", payload);
        }
        
        // Packet transmission to server
        n = sendto(sock, packet, sizeof(header) + strlen(payload) + 2, 0, (struct sockaddr *)&server, length);

        // Prints error message, if any
        if (n < 0)
        {
            error("Client: Packet transmission failed\n");
        }

        bzero(server_reply, 16);                                                            // To set all bytes in server_reply till 16 as zeroes
        n = recvfrom(sock, server_reply, 16, 0, (struct sockaddr *)&frm_client, &length);   // To receive server response

        // In case of successfull reception of response, copies it intyo server_reply and compares the ack and prints
        // In case of rejecetion, copies it and prints corresponding error type 
        if (n >= 0) 
        {
            memcpy(&acknowledge, server_reply, sizeof(acknowledge));        
            if (acknowledge.ack == 0xFFF2)
            {
                printf( "ACK received\n " );
                print_ack();
                break;
            }
            else
            {
                memcpy(&reject, server_reply, sizeof(reject));
                printf( "Rejected packet received\n" );
                printf( "Type of Error: " );
                if (reject.rejectCode == 0xFFF4)
                {
                    printf( "Packet not in sequence\n");
                }
                else if (reject.rejectCode == 0xFFF5)
                {
                    printf( "Length mismatch in Packet\n");
                }
                else if (reject.rejectCode == 0xFFF6)
                {
                    printf( "Missing End of packet\n");
                }
                else if (reject.rejectCode == 0xFFF7)
                {
                    printf( "Duplicate packet.\n");
                }
                else
                {
                    printf( "Error unknown\n");
                }
                print_rejected();
                count++;
                break;
            }
        }

        // To print retry message
        else
        {
            if(count>0){
            printf("Error: Server does not respond after 3 seconds. Trying again\n");
            }
        }
        count--;
    }

    // To print if server does not respond in three attempts
    if (count < 0)
    {
        error( "Error: Server does not respond in three attempts\n" );
    }
}
int main(int argc, char *argv[])
{

    //To check CLA, will generate error if CLA is not as required by Program and will exit from the program
    //Here the arguments will be ./"xyz" localhost and port number on which the server will LISTEN
    if (argc != 3)
    {
        printf("Type: ./client server_name port_number(localhost 8080)\n");
        exit(1);
    }

    //Socket creation at client
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
    {
        error("Client: Failed to create socket.\n");
    }

    server.sin_family = AF_INET;                //Setting address family to IPV4
    host_info = gethostbyname(argv[1]);         //Getting hostname from CLA

    if (host_info == 0)
    {
        error("Client: Host not known\n");
    }

    bcopy((char *)host_info->h_addr, (char *)&server.sin_addr, host_info->h_length);        //Copies IP address of host to server to allow connection
    server.sin_port = htons(atoi(argv[2]));             //To convert host port number to big-endian network byte order
    length = sizeof(struct sockaddr_in);

    //ACK timer set up of 3 seconds
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

    // To transmit 5 packets without any errors and receive ACK from server
    printf( "\n\nTest Case 1: Transmission of 5 packets without errors and receiving ACK from server\n\n" );
    packet_num = 1;

    while(packet_num <= 5){
        //Packet creation
        bzero(payload, 256);                        //To fill 256 bytes of payload with zeroes
        memcpy(payload, "Payload\n", 34);           //Copies "Payload" in payload buffer
        packet_builder(packet_num, payload);        //Calls function to create packet
        send_packet();                              //Transmits packet to server
        packet_num++;                               //Counter to create 5 packets
    }
    printf( "\n\n\n\nResetting packet counter to test next case\n\n\n\n" );

    printf( "\n\nTest Case 2: Transmission of 1 error free packet and 4 packets with assigned errors\n\n" );

    //Packet 1 without any error
    printf( "Packet 1: Error free packet\n" );
    bzero(payload, 256);
    memcpy(payload, "Payload\n", 34);
    packet_builder(1, payload);
    send_packet();

    //Packet 2 with duplication error
    printf( "Packet 2: Simulation of 'Duplicate Packet' error\n" );
    bzero(payload, 256);
    memcpy(payload, "Payload\n", 34);
    packet_builder(1, payload);
    send_packet();

    //Packet 3 with packet out of sequence
    printf( "Packet 3: Simulation of 'Out of Sequence' error\n" );
    bzero(payload, 256);
    memcpy(payload, "Payload\n", 34);
    packet_builder(3, payload);
    send_packet();

    //Packet 4 with mismatch in packet length
    printf( "Packet 4: Simulation of 'Length mismatch' error\n" );
    bzero(payload, 256);
    memcpy(payload, "Payload\n", 34);
    header.seg_No = 2;
    header.length = 0x44;
    memset(packet, 0, 264);
    memcpy(packet, &header, sizeof(header));
    memcpy(packet + sizeof(header), payload, strlen(payload));
    memcpy(packet + sizeof(header) + strlen(payload), &trailer, 2);
    printf("Packet created");
    print_packet();
    send_packet();

    //Packet 5 with end of packet missing
    printf( "Packet 5: Simulation of 'End of packet missing' error\n" );
    bzero(payload, 256);
    memcpy(payload, "Payload\n", 34);
    packet_builder(2, payload);
    trailer.end_ID = 0xFFF0;
    memcpy(packet + sizeof(header) + strlen(payload), &trailer, 2);
    printf( "End of packet changed " );
    print_packet();
    send_packet();
}
