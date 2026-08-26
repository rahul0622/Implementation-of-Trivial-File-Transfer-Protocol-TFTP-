/* Common file for server & client */

#include "tftp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include<fcntl.h>

/*void send_file(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, char *filename)
  {
// Implement file sending logic here
}*/
extern char mode[10];
void send_file(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, char *filename)
{
	tftp_packet packet;
	tftp_packet ackpkt;

	int fd = open(filename, O_RDONLY);
	if (fd < 0)
        {
                perror("open");
                return;
        }
	ssize_t ret = 0;

	struct sockaddr_in from;
	socklen_t fromlen = sizeof(from);

	uint16_t block = 1;


	do
	{
		//ret = read(fd, packet.body.data_packet.data, 512);
		/* For the normal mode */
		if (strcasecmp(mode, "normal") == 0)
		{
			ret = read(fd, packet.body.data_packet.data, 512);
		}

		/* For the octet mode */
		else if (strcasecmp(mode, "octet") == 0)
		{
			ret = read(fd, packet.body.data_packet.data, 1);
		}

		/* For the netascii mode */
		else if (strcasecmp(mode, "netascii") == 0)
		{
			char buff[512];
			int i = 0;
			char ch;

			while (i < 512 && read(fd, &ch, 1) == 1)
			{
				if (ch == '\n')
					buff[i++] = '\r';

				if (i == 512)
					break;

				buff[i++] = ch;
			}

			memcpy(packet.body.data_packet.data, buff, i);
			ret = i;
		}

		packet.opcode = htons(3); // DATA
		packet.body.data_packet.block_number = htons(block);

		sendto(sockfd, &packet, 4 + ret, 0,
				(struct sockaddr *)&client_addr, sizeof(client_addr));

		printf("Set Block %d, bytes = %ld\n",block,ret);

		// Wait for ACK
		recvfrom(sockfd, &ackpkt, sizeof(ackpkt), 0,
				(struct sockaddr *)&from, &fromlen);

		// Check ACK
		if (ntohs(ackpkt.opcode) != 4 ||
				ntohs(ackpkt.body.ack_packet.block_number) != block)
		{
			// resend same block
			lseek(fd, -ret, SEEK_CUR);
			continue;
		}

		block++;

	} while (ret == 512);
	printf("\n***** FILE SENT SUCCESSFULLY *****\n");
	close(fd);
}



void receive_file(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, char *filename)
{
	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0)
        {
                perror("open");
                return;
        }

	tftp_packet packet;
	tftp_packet ackpkt;

	ssize_t recvlen;
	ssize_t data_len;

	uint16_t expected_block = 1;

	do
	{
		recvlen = recvfrom(sockfd, &packet, sizeof(packet), 0,
				(struct sockaddr *)&client_addr, &client_len);

		data_len = recvlen - 4;

		if (ntohs(packet.opcode) != 3)
			continue;

		uint16_t block = ntohs(packet.body.data_packet.block_number);

		if (block == expected_block)
		{
			//write(fd, packet.body.data_packet.data, data_len);
			/* For the normal and netascii mode */
			if (strcasecmp(mode, "normal") == 0 || strcasecmp(mode, "netascii") == 0)
				write(fd, packet.body.data_packet.data, data_len);

			/* For the octet mode */
			else if (strcasecmp(mode, "octet") == 0)
				write(fd, packet.body.data_packet.data, data_len);

			expected_block++;
		}

		printf("Received Block %d,bytes = %ld\n",block,data_len);

		ackpkt.opcode = htons(4); // ACK
		ackpkt.body.ack_packet.block_number = htons(block);

		sendto(sockfd, &ackpkt, 4, 0,
				(struct sockaddr *)&client_addr, sizeof(client_addr));

	} while (data_len == 512);

	printf("\n***** FILE RECEIVED SUCCESSFULLY *****\n");

	close(fd);
}

