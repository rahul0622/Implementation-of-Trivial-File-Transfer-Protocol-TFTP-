/***************************************************************
 *  NAME = SANDEEP HOSMNAI                                     *
 *  DATE = 21/02/26                                            *
 *  Project Title : TFTP Client-Server Implementation in C     *
 *                                                             *
 *  Description:                                               *
 *  This project implements a simplified Trivial File          *
 *  Transfer Protocol (TFTP) using UDP socket programming      *
 *  in C. It supports file upload and download operations      *
 *  through RRQ (Read Request) and WRQ (Write Request).        *
 *                                                             *
 *  Features:                                                  *
 *  - Client-Server communication using UDP                    *
 *  - Supports normal, octet, and netascii modes               *
 *  - File validation using system calls (open, read, write)   *
 *  - Packet-based data transfer                               *
 *  - Error handling and command validation                    *
 *                                                             *
 *  Technologies Used:                                         *
 *  - C Programming                                            *
 *  - Linux System Calls                                       *
 *  - Socket Programming (UDP)                                 *
 *                                                             *
 *                                                             *
 *                                                             *
 ***************************************************************
 */

#include "tftp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<fcntl.h>


void handle_client(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, tftp_packet *packet);

char mode[10] = "normal";
int main() {
	int sockfd;
	struct sockaddr_in server_addr, client_addr;
	socklen_t client_len = sizeof(client_addr);
	tftp_packet packet;


	// Create UDP socket
	int sock_fd = socket(AF_INET,SOCK_DGRAM,0);

	if(sock_fd == -1)
	{
		perror("socket");
		return -1;
	}

	// Set up server address

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr =inet_addr("127.0.0.1");

	// Bind the socket
	bind(sock_fd,(struct sockaddr*)&server_addr,sizeof(server_addr));


	printf("TFTP Server listening on port %d...\n", PORT);

	// Main loop to handle incoming requests
	while (1) {
		int n = 0;
		n = recvfrom(sock_fd,&packet, sizeof(packet), 0, (struct sockaddr *)&client_addr, &client_len);
		//	printf("%d\n",n);
		if (n < 0) {
			perror("Receive failed or timeout occurred");
			continue;
		}

		printf("File name received -> %s\n", packet.body.request.filename);

		handle_client(sock_fd, client_addr, client_len, &packet);
	}

	close(sock_fd);
	return 0;
}

void handle_client(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, tftp_packet *packet) 
{
	// Extract the TFTP operation (read or write) from the received packet
	// and call send_file or receive_file accordingly
	// updating the values of mode and pack_number //
	strcpy(mode, packet->body.request.mode);

	if(packet->opcode == WRQ)
	{
		// checking the file is present or not //
		int fd = open(packet->body.request.filename,O_CREAT | O_EXCL,0644);
		if(fd == -1)
		{
			// closing the file //
			close(fd);
			fd = open(packet->body.request.filename, O_WRONLY | O_TRUNC);

		}

		packet->opcode = ACK;
		sendto(sockfd,packet,sizeof(*packet),0,(struct sockaddr*)&client_addr,sizeof(client_addr));

		// calling the receive file function //
		receive_file(sockfd,client_addr,client_len,packet->body.request.filename);

		close(fd);

	}
	else if(packet->opcode == RRQ)
	{
		printf("File name received -> %s\n",
				packet->body.request.filename);

		// Open file in READ ONLY mode
		int fd = open(packet->body.request.filename, O_RDONLY);

		if(fd == -1)
		{
			// File NOT found
			printf("File not present in server\n");

			memset(packet, 0, sizeof(*packet));
			packet->opcode = ERROR;
			packet->body.error_packet.error_code = 1;
			strcpy(packet->body.error_packet.error_msg,
					"The file Not Found...");

			sendto(sockfd, packet, sizeof(*packet), 0,
					(struct sockaddr*)&client_addr, client_len);

			return;   // stop execution
		}

		// File exists → send file
		close(fd);

		send_file(sockfd, client_addr, client_len,
				packet->body.request.filename);
	}

}




