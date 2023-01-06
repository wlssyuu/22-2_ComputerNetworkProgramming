// 2021113878 jeyujin
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

#define BUF_SIZE 2048
void error_handling(char *m);

int main(int argc, char *argv[]) {
	int serv_sock, clnt_sock, sender, receiver;
	int buf_len, fd_num, fd_max;
	struct sockaddr_in serv_adr, clnt_adr;
	struct timeval timeout;
	char buf[BUF_SIZE];
	socklen_t adr_sz;
	fd_set reads, cpy_reads;
	
	if(argc != 2) {
		printf("Usage: %s <port>\n", argv[0]);
		exit(1);
	}

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");

	if(listen(serv_sock, 5) == -1)
		error_handling("listen() error");

	FD_ZERO(&reads);
	FD_SET(serv_sock, &reads);
	fd_max = serv_sock; 

	while(1) {
		cpy_reads = reads;
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000;

		if((fd_num = select(fd_max + 1, &cpy_reads, 0, 0, &timeout)) == -1)
			break;
		if(fd_num == 0) 
			continue;
			
		if(FD_ISSET(serv_sock, &cpy_reads)) { // connect
			int menu = 0;
			adr_sz = sizeof(clnt_adr);
			clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
			FD_SET(clnt_sock, &reads);
			
			if(fd_max < clnt_sock) fd_max = clnt_sock;
			
			printf("connected client: %d\n", clnt_sock);
			read(clnt_sock, &menu, sizeof(int));
			
			if(menu == 1) sender = clnt_sock;
			else receiver = clnt_sock;
		}
		else if (FD_ISSET(sender, &cpy_reads)) { // client : sender
			buf_len = read(sender, buf, sizeof(buf));
			if(buf_len == 0) {
				FD_CLR(sender, &reads);
				close(sender);
				printf("closed client: %d\n", sender);
			}
			else {
				write(receiver, buf, buf_len);
				printf("Forward\t [%d] --> [%d]\n", sender, receiver);
			}
		}
		else if (FD_ISSET(receiver, &cpy_reads)) { // client : receiver
			memset(buf, 0, BUF_SIZE);
			buf_len = read(receiver, buf, sizeof(buf));
			if(buf_len == 0) {
				FD_CLR(receiver, &reads);
				close(receiver);
				printf("closed client: %d\n", receiver);
			}
			else {
				write(sender, buf, buf_len);
				printf("Backward [%d] <-- [%d]\n", sender, receiver);
			}
		}
	}
	
	close(serv_sock);
	return 0;
}

void error_handling(char *m) {
	fputs(m, stderr);
	fputc('\n', stderr);
	exit(1);
}
