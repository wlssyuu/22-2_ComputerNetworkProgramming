#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define BUF_SIZE 100
void error_handling(char* message);

int main(int argc, char* argv[]) {
	int fdForRead, fdForWrite;
	int readSize, writeSize, bytes = 0;
	char buf[BUF_SIZE];

	if(argc != 3) error_handling("[Error] mymove Usage: ./mymove src_file dest_file");
	
	// open src_file
	fdForRead = open(argv[1], O_RDONLY);
	if(fdForRead == -1) error_handling("open() error!\n");
	
	// open dest_file
	fdForWrite = open(argv[2], O_CREAT|O_WRONLY|O_TRUNC, 0644);
	if(fdForWrite == -1) error_handling("open() error!\n");

	// read src_file
	while((readSize = read(fdForRead, buf, sizeof(buf)))>0) {
		if(readSize == -1) error_handling("read() error!\n");
		
		bytes += readSize;
		// write contents
		writeSize = write(fdForWrite, buf, readSize);
		if(writeSize == -1) error_handling("write() error!\n");		
	}

	// delete src_file
	remove(argv[1]);

	close(fdForRead);
	close(fdForWrite);
	printf("move from %s to %s (bytes: %d) finished.\n", argv[1], argv[2], bytes);
	return 0;
}

void error_handling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
