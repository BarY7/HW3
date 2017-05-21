/*
 * Counter.c

 *
 *  Created on: May 20, 2017
 *      Author: user
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
//#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <signal.h>


#define OP_ERR "problem opening the file: %s/n"
#define WE_ERR "problem writing to file: %s/n"
int main(int argc, char** argv){
	int counter = 0;
	char counc = argv[1][0];
	char* filename = argv[2];
	off_t offset = atoll(argv[3]);
	ssize_t length = atoll(argv[4]); //TODO maybe just atol
	int fd = open(filename, O_RDWR | O_CREAT);
	if (fd == -1) {
		printf("Error opening file for writing: %s\n", strerror(errno));
		return -1;
	}

    char* arr = (char*)mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset); //check protetctiond TODO
    if (arr == MAP_FAILED) {
		printf("Error mmapping the file: %s\n", strerror(errno));
		return -1;
    }

    for(int i=0; i<length; i++){
    	if(arr[i] == counc){
    		counter++;
    	}
    }

    pid_t proid = getpid();
    pid_t ppid = getppid();
    char* pipeName = malloc(14 + sizeof(pid_t));//TODO ok?
    sprintf(pipeName, "//tmp//counter_%d" , (int) proid); //TODO double //?
    size_t fdPipe = mkfifo(pipeName, O_WRONLY);
    if(fdPipe < 0){
    	printf(OP_ERR, strerror( errno ));
    	return errno;
    }

      //TODO scary...
      kill(ppid, SIGUSR1);
      size_t wrote = write(fdPipe,&counter , 1 );
      if(wrote < sizeof(counter)){
    	  printf("didnt write all");
    	  return -1;
      }
      if(wrote < 0){
      	printf(OP_ERR, strerror( errno ));
      	return errno;
      }
      sleep(1);

      munmap(arr, length);
      close(fd);
      close(fdPipe);
      unlink(pipeName);
      free(pipeName);
      return 1;
}
