#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define FILE_REQ 0
#define FILE_SENDING 1
#define FILE_END 2
#define FILE_END_ACK 3

#define BASIC 1
#define STANDARD 2
#define PREMIUM 3

#define BASIC_BUF 10
#define STANDARD_BUF 100
#define PREMIUM_BUF 1000

#define MAX_SIZE PREMIUM_BUF
typedef struct {
	int command;
	int type;
	char buf[MAX_SIZE];
	int len; 
} PACKET;

PACKET p;

void error_handling(char *msg);
void * recv_packet(void * arg);
void * send_packet(void * arg);

int main(int argc, char *argv[]) {
	int sock, type, menu;
	struct sockaddr_in serv_addr;
	pthread_t rcv_thread;
	void * thread_return;
	
	if(argc != 3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	 }
	
	sock = socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));
	  
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error");

	// selecting user type
	while(1) {
		printf("-----------------------------------------------\n");
		printf("\t\tK-OTT Service \n");
		printf("-----------------------------------------------\n");		
		printf(" Choose a subscribe type \n");
		printf("-----------------------------------------------\n");
		printf("1: Basic, 2: Standard, 3: Premium, 4: quit: ");
		scanf("%d", &type);
		printf("-----------------------------------------------\n");
		
		switch(type) {
		case 1: 
			p.type = BASIC;
			break;
		case 2: 
			p.type = STANDARD;
			break;
		case 3: 
			p.type = PREMIUM;
			break;
		case 4:
			printf("Exit Program");
			exit(0);
		}
		
		printf("1. Download, 2: Back to Main menu: ");
		scanf("%d", &menu);
		
		if(menu == 1) {
			p.command = FILE_REQ;
			printf("-----------------------------------------------\n");
			break;
		}
	}
	
	// send user type, file_req
	write(sock, &p, sizeof(PACKET));

	pthread_create(&rcv_thread, NULL, recv_packet, (void*)&sock);
	if(pthread_join(rcv_thread, &thread_return) != 0)
		error_handling("join() error");
	
	return 0;
}

void * recv_packet(void * arg) {
	int sock = *((int*)arg);
	int size, total_len = 0;
	unsigned long nano = 1000000000;
	unsigned long t1, t2;
	struct timespec start, end;
	
	// receive file, print "."
	clock_gettime(CLOCK_REALTIME, &start);
	t1 = start.tv_nsec + start.tv_sec * nano;
	
	while(1) {
		size = read(sock, &p, sizeof(PACKET));
		if(size == -1) {
			return (void*) - 1;
		}
		if(p.command == FILE_SENDING) {
			total_len += p.len;
			printf(".");
		}
		if(p.command == FILE_END) {
			break;
		}
	}
	
	clock_gettime(CLOCK_REALTIME, &end);
	t2 = end.tv_nsec + end.tv_sec * nano;
	
	// send file_end_ack
	if(p.command == FILE_END) {
		p.command = FILE_END_ACK;
		write(sock, &p, sizeof(PACKET));
		printf("\nFile Transmission Finished\n");
		printf("Total received bytes: %d\n", total_len);
		printf("Downloading Time: %ld msec\n", (t2-t1)/1000000);
		close(sock);
		printf("Client closed\n");
	}
	
	return NULL;
}
	
void error_handling(char *msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
