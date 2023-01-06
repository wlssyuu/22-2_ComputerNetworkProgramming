// 2021113878 Je Yujin hw2_server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

typedef struct {
	int cmd;
	char addr[20];
	struct in_addr iaddr;
	int result;
} PACKET;

void error_handling(char *message);

int main(int argc, char *argv[]) {
	PACKET p;
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;
	int pSize;

	if(argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(serv_sock == -1) error_handling("socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));

	if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) error_handling("bind() error");

	if(listen(serv_sock, 5) == -1) error_handling("listen() error");

	clnt_addr_size = sizeof(clnt_addr);
	clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
	if(clnt_sock == -1) error_handling("accept() error");

	printf("---------------------------\n");
	printf(" Address Conversion Server \n");
	printf("---------------------------\n");
	
	while(1) {
		pSize = read(clnt_sock, &p, sizeof(PACKET));
		if(pSize == -1) error_handling("read() error!");
		
		if(p.cmd == 2) {
			printf("[Rx] QUIT meesageg received\n");
		   
			break;
		}
		else {
			printf("[Rx] Received  Dotted-Demical Address: %s\n", p.addr);
			p.cmd = 1;
			
			if((p.result = inet_aton(p.addr, &p.iaddr)) == 1) {
				printf("inet_aton(%s) -> %#x\n", p.addr, p.iaddr.s_addr);
				printf("[Tx] cmd: %d, iaddr: %#x, result: %d\n\n", p.cmd, p.iaddr.s_addr, p.result);
			}
			else printf("[Tx] Address conversion fail: (%s)\n\n", p.addr);
			
			write(clnt_sock, &p, sizeof(PACKET));
		}
	}
		
	close(clnt_sock);
	close(serv_sock);

	printf("Server socket close and exit.\n");
	
	return 0;
}

void error_handling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
