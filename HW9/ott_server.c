#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
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

#define MAX_CLNT 256
#define MAX_SIZE PREMIUM_BUF
typedef struct {
	int command;
	int type;
	char buf[MAX_SIZE];
	int len; 
} PACKET;

PACKET p;
void error_handling(char * msg);
void * handle_clnt(void * arg);
int send_packet(int clnt_sock, int buf_len);

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;

int main(int argc, char *argv[]) {
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;
	
	if(argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	
	printf("------------------------\n");
	printf("  K-OTT Service Server\n");
	printf("------------------------\n");
	
	pthread_mutex_init(&mutx, NULL);
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET; 
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");
	if(listen(serv_sock, 5) ==-1)
		error_handling("listen() error");
	
	while(1) {
		clnt_adr_sz = sizeof(clnt_adr);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
				
		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++] = clnt_sock;
		pthread_mutex_unlock(&mutx);

		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(t_id);
		printf("Connected client IP: %s , clnt_sock = %d\n", inet_ntoa(clnt_adr.sin_addr), clnt_sock);
	}
	
	close(serv_sock);
	return 0;
}
	
void * handle_clnt(void * arg) {
	int clnt_sock = *((int*)arg);
	int buf_len, total_len;
	char type[10];

	// receive user type, command = file_req
	read(clnt_sock, &p, sizeof(PACKET));
	
	if(p.command == FILE_REQ) {
		if(p.type == BASIC) {
			buf_len = BASIC_BUF;
			strcpy(type, "Basic");
		}
		else if(p.type == STANDARD) {
			buf_len = STANDARD_BUF;
			strcpy(type, "Basic");
		}
		else {
			buf_len = PREMIUM_BUF;
			strcpy(type, "Premium");
		}
	}
	
	// send file, command = file_sending
	total_len = send_packet(clnt_sock, buf_len);
	
	// receive command = file_end_ack
	read(clnt_sock, &p, sizeof(PACKET));
	
	pthread_mutex_lock(&mutx);
	if(p.command == FILE_END_ACK) {
		for(int i = 0; i < clnt_cnt; i++) { // remove disconnected client
			if(clnt_sock == clnt_socks[i]) {
			printf("\nTotal Tx Bytes: %d to Client %d (%s)\n", total_len, clnt_sock, type);
			printf("[Rx] FILE_END_ACK from Client %d => clnt_sock: %d closed.\n", clnt_sock, clnt_sock);
				while(i < clnt_cnt) {
					clnt_socks[i] = clnt_socks[i + 1];
					i++;
				}
				break;
			}
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	
	close(clnt_sock);
	
	return NULL;
}

int send_packet(int clnt_sock, int buf_len) {
	int total_len = 0, rd;
	
	rd = open("hw06.mp4", O_RDONLY);
	while((p.len = read(rd, p.buf, buf_len)) >= 0) {
		p.command = FILE_SENDING;
		total_len += p.len;
		// printf("%d ", total_len);
		if(p.len < buf_len) {
			write(clnt_sock, &p, sizeof(PACKET));
			break;
		}
		write(clnt_sock, &p, sizeof(PACKET));
	}
	close(rd);
	
	p.command = FILE_END;
	write(clnt_sock, &p, sizeof(PACKET));
	
	return total_len;
}

void error_handling(char * msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
