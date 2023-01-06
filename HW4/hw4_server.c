// hw4_server.c  2021113878 Je YuJin

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 100
#define SEQ_START 1000
typedef struct {
	int seq;
	int ack;
	int buf_len;
	char buf[BUF_SIZE];
} PACKET;

void error_handling(char *m);

int main(int argc, char *argv[]) {
	PACKET p;
	int serv_sock, clnt_sock;
	int pSize, rd, file_len = 0;
	char file_name[BUF_SIZE];

	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;
	
	if(argc !=  2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	printf("------------------------------\n");
	printf("   File Transmission Server\n");
	printf("------------------------------\n");

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(serv_sock == -1) error_handling("socket() error!");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));

	if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1) error_handling("bind() error!");

	if(listen(serv_sock, 5) == -1) error_handling("listen() error");

	clnt_addr_size = sizeof(clnt_addr);

	clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_addr, &clnt_addr_size);
	
	if(clnt_sock == -1) error_handling("accept() error");
	
	pSize = read(clnt_sock, &p, sizeof(PACKET));
	if(pSize == -1) error_handling("read() error");
	strcpy(file_name, p.buf);
	
	rd = open(file_name, O_RDONLY);
	if(rd == -1) {
		printf("%s File Not Found\n", file_name);
		strcpy(p.buf, "File Not Found");
		p.buf_len = -1;
		write(clnt_sock, &p, sizeof(PACKET));
		}
	else {
		printf("[Server] sending %s\n\n", file_name);
		p.seq = SEQ_START;
		
		while((p.buf_len = read(rd, p.buf, sizeof(p.buf)))>=0) {
			write(clnt_sock, &p, sizeof(PACKET));
			printf("[Server] Tx: SEQ: %d, %d byte data\n", p.seq, p.buf_len);

			file_len += p.buf_len;
			
			if(p.buf_len < BUF_SIZE) {
				write(clnt_sock, &p, sizeof(PACKET));
				break;
			}

			pSize = read(clnt_sock, &p, sizeof(PACKET));
			if(pSize == -1) error_handling("read() error");
			printf("[Server] Rx ACK: %d\n\n", p.ack);
			
			p.seq = p.ack;
		}

		printf("%s sent (%d bytes)\n", file_name, file_len);
	}
	
	close(clnt_sock);
	close(serv_sock);
	close(rd);
	
	return 0;
}

void error_handling(char *m) {
	fputs(m, stderr);
	fputc('\n', stderr);
	exit(1);
}
