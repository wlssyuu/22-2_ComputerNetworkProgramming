// 2021113878 Je Yujin hw2_client.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void error_handling(char *message);

typedef struct {
	int cmd;
	char addr[20];
	struct in_addr iaddr;
	int result;
} PACKET;


int main(int argc, char *argv[]) {
	PACKET p;
	int sock;
	struct sockaddr_in serv_addr;
	char message[30];
	int pSize;
	
	if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if(sock == -1) error_handling("socket() error");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));
		
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) error_handling("connect() error!");
			
	while(1) {
		printf("Input dotted-demical address: ");
		scanf("%s", p.addr);
		
		if(strcmp(p.addr, "quit") == 0) {
			p.cmd = 2; // cmd 2 : quit
			printf("[Tx] cmd: %d(QUIT)\n", p.cmd);
			write(sock, &p, sizeof(PACKET));
			
			break;
		}
		p.cmd = 0;
		printf("[Tx] cmd: %d, addr: %s\n", p.cmd, p.addr);
		
		write(sock, &p, sizeof(PACKET));
		
		pSize = read(sock, &p, sizeof(PACKET));
		if(pSize == -1) error_handling("read() error!");
	
		if(p.result == 1) printf("[Rx] cmd: %d, Address conversion: %#x (result: %d)\n\n", p.cmd, p.iaddr.s_addr, p.result);
		else printf("[Rx] cmd: %d, Address conversion fail! (result: %d)\n\n", p.cmd, p.result);
	}
	
	close(sock);
	printf("Client socket close and exit\n");
	
	return 0;
}

void error_handling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
