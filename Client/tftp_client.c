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
#include "tftp_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include<fcntl.h>


int flag_connection = 0;
char mode[10] = "normal";
int main() {
	char command[256];
	tftp_client_t client;

	memset(&client, 0, sizeof(client));  // Initialize client structure

	// Main loop for command-line interface
	while (1) {
		printf("TFTP > \n");
		printf("1. Connect <ip>\n2. Get <filename>\n3. Put <filename>\n4. Mode <octet/netascii>\n5. Exit\n\n");

		printf("tftp]:\t\t");
		fgets(command, sizeof(command), stdin);

		// Remove newline character
		command[strcspn(command, "\n")] = 0;

		// Process the command
		process_command(&client, command);
	}

	return 0;
}

// Function to process commands
void process_command(tftp_client_t *client, char *command) 
{


	char ip[20];
	if(strcasecmp(command,"Connect") == 0)
	{
		printf("Enter the IP addr:");
		fgets(ip, sizeof(ip), stdin);
		ip[strcspn(ip, "\n")] = 0;
		if(validate_ip(ip) == 0)
		{
			connect_to_server(client,ip,PORT);
			flag_connection = 1;
		}
		else
		{
			printf("Error: Invalid IP address format!!\nExample: 127.0.0.1\n");
			return;
		}


	}
	else if(strcasecmp(command,"Get")==0)
	{
		if(flag_connection == 1)
		{
			char filename[20];
			printf("Enter the filename you want to recieve: ");
			fgets(filename, sizeof(filename), stdin);
			filename[strcspn(filename, "\n")] = 0;
			get_file(client,filename);
		}
		else
		{
			printf("Connection Not Established.....\n\n");
		}

	}
	else if(strcasecmp(command,"Put") == 0)
	{
		if(flag_connection == 1)
		{
			char filename[20];
			printf("Enter the filename:");
			fgets(filename, sizeof(filename), stdin);
			filename[strcspn(filename, "\n")] = 0;


			int fd = open(filename,O_CREAT | O_EXCL,0644);
			
			if(fd == -1)
			{

				put_file(client,filename);
				close(fd);




			}
			else
			{
				printf("Error: Given file is not present!!!\n");
				return;
			}
		}
		else
		{
			printf("Connection Not Established.....\n\n");
		}





	}
	else if(strcasecmp(command,"Exit")==0)
	{
		exit(0);
	}
	else if(strcasecmp(command,"mode")==0)
	{
		char *menu[] = {"normal", "octet", "netascii"};
                int size = sizeof(menu) / sizeof(menu[0]);

		printf("MENU:\n1. Normal\n2. Octet\n3. Netascii\n\n");
		printf("Enter the Option:");
		fgets(mode, sizeof(mode), stdin);
		mode[strcspn(mode, "\n")] = '\0';
                
		int valid = 0;
		// Compare input with menu array
		for(int i = 0; i < size; i++)
		{
			if(strcmp(mode, menu[i]) == 0)
			{
				valid = 1;
				break;
			}
		}

		if(valid)
		{
			printf("Corrent Mode Selected: %s\n", mode);
		}
		else
		{
			printf("Invalid option\n");
			printf("Please choose the correct option\n\n");
		}



	}
	else
	{
		printf("Error: invalid command.....\n\n");
		return;
	}



}

int validate_ip(char *ip)
{
	struct sockaddr_in sa;
	return inet_pton(AF_INET, ip, &(sa.sin_addr)) == 1 ? 0 : -1;
}


// This function is to initialize socket with given server IP, no packets sent to server in this function
void connect_to_server(tftp_client_t *client, char *ip, int port) {
	// Create UDP socket
	client->sockfd = socket(AF_INET,SOCK_DGRAM,0);

	if(client->sockfd == -1)
	{
		perror("socket");
		return ;
	}
        
	// Set up server address
	client->server_addr.sin_family = AF_INET;
	client->server_addr.sin_port = htons(port);
	client->server_addr.sin_addr.s_addr =inet_addr(ip); 
	client->server_len = sizeof(client->server_addr);

	printf("Cannection Established....\n");

}

void put_file(tftp_client_t *client, char *filename)
{
	// Send WRQ request and send file
	send_request(client->sockfd,client->server_addr,filename,WRQ);

}

void get_file(tftp_client_t *client, char *filename) 
{
	// Send RRQ and recive file
	receive_request(client->sockfd,client->server_addr,filename,RRQ); 

}

void disconnect(tftp_client_t *client) 
{
	// close fd
	close(client -> sockfd);

}
void send_request(int sockfd,struct sockaddr_in server_addr, char *filename, int opcode)
{
	tftp_packet pck;
	// clearing the garbage values //
	memset(&pck, 0, sizeof(pck));

	// initilizing the variables
	strcpy(pck.body.request.filename, filename);

	pck.opcode = opcode;
	socklen_t server_len = sizeof(server_addr);
	strcpy(pck.body.request.mode, mode);

	// sending request to the server //
	sendto(sockfd,&pck, sizeof(pck),0,(struct sockaddr*)&server_addr,sizeof(server_addr));

	struct sockaddr_in from;
	socklen_t fromlen = sizeof(from);

	// reciving the ack from the server_side
	recvfrom(sockfd, &pck, sizeof(pck), 0, (struct sockaddr*)&from, &fromlen);

	if(ACK == pck.opcode)
	{
			
		printf("READY TO SEND\n");
		send_file(sockfd,from,fromlen,filename);
	}	

}

void receive_request(int sockfd,struct sockaddr_in server_addr, char *filename, int opcode)
{
	tftp_packet pck;

	// clearing the garbage values //
	memset(&pck, 0, sizeof(pck));
	strcpy(pck.body.request.filename, filename);
	pck.opcode = opcode;
	socklen_t server_len = sizeof(server_addr);

	sendto(sockfd,&pck, sizeof(pck),0,(struct sockaddr*)&server_addr,sizeof(server_addr));

	struct sockaddr_in from;
	socklen_t fromlen = sizeof(from);

	// receiving the acknowledgement //
	recvfrom(sockfd, &pck, sizeof(pck), 0, (struct sockaddr*)&from, &fromlen);


	if(ACK == pck.opcode)
	{
		printf("READY TO RECEIVE\n");
		receive_file(sockfd,from,fromlen,filename);
	}
	else if(ERROR == pck.opcode)
	{

		// This is for ERROR
		printf("Error\n");
		printf("%s\n",pck.body.error_packet.error_msg);
		return;
	}

}
