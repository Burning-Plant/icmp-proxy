#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PACKET_LENGTH 4096
//#define DEBUG

#pragma pack(push, 1)
typedef struct iphdr {
    uint8_t  version_ihl; // Version (4 bits) + IHL (4 bits)
    uint8_t  tos;         // Type of Service
    uint16_t totalLength; // Total Length
    uint16_t id;          // Identification
    uint16_t flag_fragOff;// Flags (3 bits) + Fragment Offset (13 bits)
    uint8_t  ttl;         // Time to Live
    uint8_t  protocol;    // Protocol
    uint16_t checksum;    // Header Checksum
    uint32_t saddr;       // Source Address
    uint32_t daddr;       // Destination Address
} iphdr;
#pragma pack(pop)

// From RFC 1071 //
u_int16_t checksum(int count, void* addr){

    /* Compute Internet Checksum for "count" bytes
    *         beginning at location "addr".
    */
    u_int8_t * ptr = (u_int8_t *) addr;
    int sum = 0;
    
    while( count > 1 ){
        sum += * (u_int16_t *) ptr++;
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
	u_int32_t source, destination;
	destination = inet_addr("8.8.8.8");
	source = inet_addr("192.168.1.198");


	char buffer[PACKET_LENGTH];
	memset(buffer, 0, PACKET_LENGTH);

	struct iphdr *ip = (struct iphdr *) buffer;
	struct icmphdr *icmp = (struct icmphdr *) (buffer + sizeof(iphdr)); //mem address starts after the iphdr
	struct sockaddr_in sin;

	int socketd = socket(PF_INET, SOCK_RAW, IPPROTO_RAW);
	if( socketd < 0) {
		perror("socket() error");
		exit(-2);
	}
	int one = 1;
	setsockopt(socketd,IPPROTO_IP, IP_HDRINCL, &one, sizeof(one));


	sin.sin_family = AF_INET;
	sin.sin_port = 0; //icmp doesnt use a port
	sin.sin_addr.s_addr = destination;

	ip->version_ihl = (4 << 4) | 5;
	ip->tos = 0;
	ip->totalLength = htons(84);//total length
	ip->id = 0;
	ip->flag_fragOff = htons((u_int16_t)(0b010 << 13) | 0); //bits are R|DF|MF
	ip->ttl = 64;
	ip->checksum = 0;
	ip->saddr = source;
	ip->daddr = destination;


	ip->checksum = checksum(ntohs(ip->totalLength),buffer);

#ifdef DEBUG
	FILE* fd = fopen("packet","w");
	if (fd < 0) {
		perror("fopen()");
		exit(-4);
	}
	fwrite(buffer, sizeof(char), PACKET_LENGTH, fd);
	fclose(fd);
#endif

	int sendError = sendto(socketd, buffer, ntohs(ip->totalLength), 0, (struct sockaddr *)&sin, sizeof(sin));
	
	if (sendError < 0) {
		perror("sendto()");
		exit(-3);
	}

	close(socketd);
	return 0;
}
