// hw4_client.c  2021113878 Je YuJin

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
	int sock, wd, file_len = 0, pSize;
	char file_name[BUF_SIZE];
	struct sockaddr_in serv_addr;

	if(argc != 3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if(sock == -1) error_handling("socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) error_handling("connect() error");

	printf("Input file name : ");
	scanf("%s", file_name);
	strcpy(p.buf, file_name);

	write(sock, &p, sizeof(PACKET));
	printf("[Client] request %s\n\n", p.buf);
	
	pSize = read(sock, &p, sizeof(PACKET));
	if(pSize == -1) error_handling("read() error");

	if(p.buf_len == -1) puts(p.buf);
	else {
		wd = open(file_name, O_CREAT|O_WRONLY|O_TRUNC, 0644);
		if(wd == -1) error_handling("open() error");
		
		while(1) {
			printf("[Client] Rx SEQ: %d, len: %d bytes\n", p.seq, p.buf_len);
			write(wd, p.buf, p.buf_len);
			file_len += p.buf_len;
			
			if(p.buf_len < BUF_SIZE) break;
			
			p.ack = p.seq + p.buf_len + 1;
			write(sock, &p, sizeof(PACKET));
			printf("[Client] Tx ACK: %d\n\n", p.ack);
			
			pSize = read(sock, &p, sizeof(PACKET));
			if(pSize == -1) error_handling("read() error");
		}

		printf("%s received (%d bytes)\n", file_name, file_len);
	}
	close(sock);
	close(wd);
	
	return 0;
}

void error_handling(char *m) {
    fputs(m, stderr);
    fputc('\n', stderr);
    exit(1);
 }
