#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define TRUE 1
#define FALSE 0
#define TTL 64
#define BUF_SIZE 128
void error_handling(char *message);

int main(int argc, char *argv[]) {
	socklen_t adr_sz;
	int send_sock, recv_sock, time_live = TTL, option, msg_len;
	char name[BUF_SIZE], buf[BUF_SIZE], message[BUF_SIZE];
	struct sockaddr_in mul_adr, adr;
	struct ip_mreq join_adr;
	socklen_t optlen;
	pid_t pid;
	
	if(argc != 4) {
		printf("Usage : %s <GroupIP> <PORT> <Name>\n", argv[0]);
		exit(1);
	}
	
	name[0] = '[';
	strcat(name, argv[3]);
	strcat(name, "] ");
	strcat(message, name);
	
	send_sock = socket(PF_INET, SOCK_DGRAM, 0);
	memset(&mul_adr, 0, sizeof(mul_adr));
	mul_adr.sin_family = AF_INET;
	mul_adr.sin_addr.s_addr = inet_addr(argv[1]);
	mul_adr.sin_port = htons(atoi(argv[2]));
	setsockopt(send_sock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&time_live, sizeof(time_live));
	
	recv_sock = socket(PF_INET, SOCK_DGRAM, 0);
	
	optlen = sizeof(option);
	option = TRUE;	
	setsockopt(recv_sock, SOL_SOCKET, SO_REUSEADDR, &option, optlen);

	memset(&adr, 0, sizeof(adr));
	adr.sin_family = AF_INET;
	adr.sin_addr.s_addr = htonl(INADDR_ANY);
	adr.sin_port = htons(atoi(argv[2]));
	
        if(bind(recv_sock, (struct sockaddr*)&adr, sizeof(adr)) == -1)
        error_handling("bind() error");
        
	join_adr.imr_multiaddr.s_addr = inet_addr(argv[1]);
	join_adr.imr_interface.s_addr = htonl(INADDR_ANY);
	setsockopt(recv_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&join_adr, sizeof(join_adr));
	
	pid = fork();
	while(1) {
		if(pid == 0) { // child, recvfrom
			msg_len = recvfrom(recv_sock, message, BUF_SIZE - 1, 0, NULL, 0);
			printf("Received Message: %.*s", msg_len, message);
		}
		else { // parent, sendto
			memset(message, 0, BUF_SIZE);
			fgets(buf, BUF_SIZE, stdin);
			if(strcmp(buf, "q\n") == 0 || strcmp(buf, "Q\n") == 0) {
				setsockopt(recv_sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void*)&join_adr, sizeof(join_adr));
				break;
			}
			
			strcat(message, name);
			strcat(message, buf);
			sendto(send_sock, message, strlen(message), 0, (struct sockaddr*)&mul_adr, sizeof(mul_adr));
		}
	}
			
	close(recv_sock);
	close(send_sock);
	printf("Leave multicast group\n");
		
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
