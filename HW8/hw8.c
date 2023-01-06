#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
	int cfd1, cfd2;
	char buf1[BUF_SIZE], buf2[BUF_SIZE];
	FILE *fp1, *fp2;
	
	cfd1 = open("data1.txt", O_RDONLY);	
	if(cfd1 == -1) {
		fputs("file open error", stdout);
		return -1;
	}
	
	cfd2 = dup(cfd1);
	if(cfd2 == -1) {
		fputs("dup error", stdout);
		return -1;
	}
	
	fp1 = fdopen(cfd1, "r");
	fp2 = fdopen(cfd2, "r");

	while (fgets(buf1, BUF_SIZE, fp1)) {
		fputs(buf1, stdout);
		fflush(fp1);
		if(fgets(buf2, BUF_SIZE, fp2)) {
		    fputs(buf2, stdout);
		    fflush(fp2);
		}
	}	
	
	fclose(fp1);
	fclose(fp2);
	
	return 0;
}
