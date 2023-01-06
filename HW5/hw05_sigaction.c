// 2021113878 JeYuJin
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

void cTimeout(int sig) {
	static int etime = 0;
	if(sig == SIGALRM) printf("[Child]  time out: 5, elapsed time: %3d seconds\n", etime+=5);
	alarm(5);
}

void pTimeout(int sig) {
	static int etime = 0;
	if(sig == SIGALRM) printf("<Parent> time out: 2, elapsed time: %3d seconds\n", etime+=2);
	alarm(2);
}

void cEnd(int sig) {
	int status;
	pid_t pid = waitpid(-1, &status, WNOHANG);
	if(WIFEXITED(status)) printf("Child id: %d, sent: %d\n", pid, WEXITSTATUS(status));
}

int main(int argc, char *argv[]) {
	struct sigaction childEnd;
	childEnd.sa_handler = cEnd;
	sigemptyset(&childEnd.sa_mask);
	childEnd.sa_flags = 0;
	sigaction(SIGCHLD, &childEnd, 0);
	
	struct sigaction act;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	pid_t pid = fork();
	
	if(pid == 0) {
		act.sa_handler = cTimeout;
		sigaction(SIGALRM, &act, 0);
		alarm(5);
		for(int i=0; i<20; i++) sleep(1);
		return 5;
	}
	else {
		act.sa_handler = pTimeout;
		sigaction(SIGALRM, &act, 0);
		alarm(2);
		while(1) sleep(1);
	}
	return 0;
}
