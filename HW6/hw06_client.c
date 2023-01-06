// 2021113878 jeyujin
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

#define BUF_SIZE 2048
void error_handling(char *m);

int main(int argc, char *argv[]) {
	int menu, fd1, fd2;
	int buf_len, fd_num, fd_max;
	char buf[BUF_SIZE];
	struct sockaddr_in serv_adr;
	struct timeval timeout;
	fd_set reads, cpy_reads;

	if(argc != 3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	printf("------------------------------\n");
	printf(" Choose function\n");
	printf(" 1. Sender 2. Receiver\n");
	printf("------------------------------\n");
	printf(" => ");
	scanf("%d", &menu);

	FD_ZERO(&reads);
			
	if(menu == 1) {
		fd1 = open("rfc1180.txt", O_RDONLY);
		FD_SET(fd1, &reads);
	}

	fd2 = socket(PF_INET, SOCK_STREAM, 0);   
	if(fd2 == -1) error_handling("socket() error");
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_adr.sin_port = htons(atoi(argv[2]));
			
	printf("\n");
	if(menu == 1) printf("File Sender Start!\n");
	else printf("File Receiver Start!\n");
	
	if(connect(fd2, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
		error_handling("connect() error!");
	else puts("Connected..........");

	FD_SET(fd2, &reads);
	fd_max = fd2;
	
	if(menu == 1) printf("fd1: %d, fd2: %d\n", fd1, fd2);
	else printf("fd2: %d\n", fd2);
	printf("fd_max: %d\n", fd_max);
	
	write(fd2, &menu, sizeof(int));
	
	while(1) {
		cpy_reads = reads;
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;

		if((fd_num = select(fd_max + 1, &cpy_reads, 0, 0, &timeout)) == -1)
		    break;
		if(fd_num == 0)
		    continue;
		
		if(menu == 1) { // sender
			if(FD_ISSET(fd2, &cpy_reads)) { // read message and print
				buf_len = read(fd2, buf, BUF_SIZE);
				write(1, buf, buf_len);
			}
			if(FD_ISSET(fd1, &cpy_reads)) { // read file and send
				buf_len = read(fd1, buf, BUF_SIZE);
				sleep(1);
				if(buf_len == 0) {
					FD_CLR(fd1, &reads);
					close(fd1);
				}
				write(fd2, buf, buf_len);
			}			
		}
		else { // receiver
			if(FD_ISSET(fd2, &cpy_reads)) {
				buf_len = read(fd2, buf, BUF_SIZE);
				write(fd2, buf, buf_len);
				write(1, buf, buf_len);
			}
		}
	}
	FD_CLR(fd2, &reads);
	close(fd2);
	return 0;
}

void error_handling(char *m) {
	fputs(m, stderr);
	fputc('\n', stderr);
	exit(1);
}
