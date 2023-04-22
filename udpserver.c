#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>


typedef struct packet{
    char data[1024]; // A packet that will contain a string of data
}Packet;

typedef struct frame{
    int frame_kind; // The kind of frame: ACK:0, SEQ:1, FIN:2
    int sq_no; // Sequence number of the frame
    int ack; // Acknowledgment number of the frame
    Packet packet; // A packet that is transmitted in the frame
}Frame;

int main(int argc, char** argv){
    // Check if the program is called with one argument: the port number
	if (argc != 2){
		printf("Usage: %s <port>", argv[0]);
		exit(0);	
	}

	int port = atoi(argv[1]); // Convert the port number string to an integer
	int sockfd; // Socket file descriptor
	struct sockaddr_in serverAddr, newAddr; // Address structures for the server and the client
	char buffer[1024]; // A buffer for receiving data
	socklen_t addr_size; // The size of the client address

	int frame_id=0; // The sequence number of the last received frame
	Frame frame_recv; // The last received frame
	Frame frame_send; // The acknowledgment frame

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	
	memset(&serverAddr, '\0', sizeof(serverAddr)); // Initialize the server address structure with zeros
	serverAddr.sin_family = AF_INET; // Set the address family to IPv4
	serverAddr.sin_port = htons(port); // Set the port number
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Set the IP address of the server

	bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)); // Bind the socket to the server address
	addr_size = sizeof(newAddr); // Get the size of the client address structure

	while(1){
		int f_recv_size = recvfrom(sockfd, &frame_recv, sizeof(Frame), 0, (struct sockaddr*)&newAddr, &addr_size); // Receive a frame from a client
		if (f_recv_size > 0 && frame_recv.frame_kind == 1 && frame_recv.sq_no == frame_id){  // If the frame is a data frame and has the expected sequence number
			printf("[+]Frame Received: %s\n", frame_recv.packet.data); // Print the data of the received frame
			 if (strcmp(frame_recv.packet.data,"exit")==0){ // If the data contains the word "exit"
             exit(1); // Terminate the program
            }
			
			frame_send.sq_no = 0; // Set the sequence number of the acknowledgment frame
			frame_send.frame_kind = 0; // Set the kind of the acknowledgment frame to ACK
			frame_send.ack = frame_recv.sq_no + 1; // Set the acknowledgment number of the acknowledgment frame
			sendto(sockfd, &frame_send, sizeof(frame_send), 0, (struct sockaddr*)&newAddr, addr_size); // Send the acknowledgment frame to the client
			printf("[+]Ack Send\n"); // Print a message that the acknowledgment
		}else{
			printf("[+]Frame Not Received\n");
		}
		frame_id++;	
	}
	
	close(sockfd);
	return 0;
}