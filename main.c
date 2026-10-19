#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PACKET_LENGTH 4096

typedef struct iphdr {
	unsigned int version : 4;
	unsigned int ihl : 4;
	unsigned int tos : 8;
	unsigned int totalLength : 16;

	unsigned int id : 16;
	unsigned int flags : 3;
	unsigned int fragmentOffset : 13;

	unsigned int ttl : 16;
	unsigned int protocol : 16;
	unsigned int checksum : 16;

	unsigned int sourceAddr : 32;

	unsigned destAddr : 32;
}iphdr;

// From RFC 1071 //
u_int16_t checksum(int count, void* addr){

    /* Compute Internet Checksum for "count" bytes
    *         beginning at location "addr".
    */
    
    int sum = 0;
    
    while( count > 1 ){
        sum += * (u_int16_t *) addr++;
        count -= 2;
    }
    
    /* Add the left-over byte, if any */
    if( count > 0 ){
        sum += * (u_int8_t *)addr;
    }
    
    /* Fold 32-bit sum to 16 bits */
    while( sum >> 16 ){
        sum = (sum & 0xffff) + (sum >> 16);
    }
    
    return (u_int16_t)(~sum);
}



int main(void) {

	u_int16_t src_port, dest_port;
	u_int32_t src_addr, dest_addr;
	dest_addr = inet_addr("8.8.8.8");
	src_addr = inet_addr("192.168.1.186");


	char buffer[PACKET_LENGTH];
	memset(buffer, 0, PACKET_LENGTH);

	struct iphdr *ip = (struct iphdr *) buffer;
	struct icmphdr *icmp = (struct icmphdr *) (buffer + sizeof(iphdr)); //mem address starts after the iphdr
	struct sockaddr_in sin;

	int socketd = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
	if( socketd < 0) {
		perror("socket() error");
		exit(-2);
	}


	sin.sin_family = AF_INET;
	sin.sin_port = 9900; //icmp doesnt use a port
	sin.sin_addr.s_addr = dest_addr;

	ip->version = 4;
	ip->ihl = 5;
	ip->tos = 0;
	ip->totalLength = 84;//total length
	ip->id = 0;
	ip->flags = 0b010; //bits are R|DF|MF
	ip->fragmentOffset = 0;
	ip->ttl = 64;
	//checksum
	ip->sourceAddr = src_addr;
	ip->destAddr = dest_addr;


	ip->checksum = checksum(ip->totalLength,buffer);

	FILE* fd = fopen("packet","w");
	if (fd < 0) {
		perror("fopen()");
		exit(-4);
	}
	fwrite(buffer, sizeof(char), PACKET_LENGTH, fd);
	fclose(fd);

	int sendError = sendto(socketd, buffer, ip->totalLength, 0, (struct sockaddr *)&sin, sizeof(sin));
	
	if (sendError < 0) {
		perror("sendto()");
		exit(-3);
	}

	close(socketd);
	return 0;
}
