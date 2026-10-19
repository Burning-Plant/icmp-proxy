#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PACKET_LENGTH 4096

typedef struct = {
	u_int4_t version;
	u_int4_t ihl;
	u_int8_t tos;
	u_int16_t len;
	u_int32_t id;
	char flags;


}iphdr;

int main(void) {

	u_int16_t src_port, dst_port;
	u_int32_t src_addr, dst_addr;
	dst_addr = inet_addr("8.8.8.8");
	src_addr = inet_addr("127.0.0.1");


	char buffer[PACKET_LENGTH];
	memset(buffer, 0, PCKT_LEN);

	struct iphdr *ip = (struct iphdr *) buffer;
	struct icmphdr *icmp = (struct icmphdr *) (buffer + sizeof(struct iphdr)); //mem address starts after the iphdr
	struct sockaddr_in sin;


	int socketd = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
	if( socketd < 0) {
		perror("socket() error");
		exit(-2);
	}


	sin.sin_family = AF_INET;
	sin.sin_port = 0; //icmp doesnt use a port
	sin.sin_addr.s_addr = dst_addr;

	int sendError = sendto(socketd, buffer, ip->tot_len, 0, (struct sockaddr *)&sin, sizeof(sin));
	
	if (sendError < 0) {
		perror("sendto()");
		exit(-3);
	}

	close(socketd);
	return 0;
}
