/*
 * Disaptch.c
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
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

int COUNT = 0;

void my_signal_handler( int signum, siginfo_t* info, void* ptr)
{
	int sonPid = info->si_pid;
    char* pipeName = malloc(14 + sizeof(int));//TODO ok?
    sprintf(pipeName, "//tmp//counter_%d" , (int) sonPid); //TODO double //?
    size_t fdPipe = open(pipeName, O_RDONLY);
    char* cRead = malloc(sizeof(int));
    int countRead = read(fdPipe,cRead,1);
    if(countRead < 0){
		printf("Read has failed on the file: %s\n", strerror( errno )); //TODO trouble?
		return;
    }
    int amount = atoi(cRead);
    COUNT+= amount;
    return;
}

int main(int argc, char** argv){
	if(argc != 3){
		printf("Wrong arguments");
		return -1;
	}
	char ccount = argv[1][0];
	char* filename = argv[2];
	struct stat sb;
	if(stat(filename,&sb)<0){
		{
			printf("Stat has failed on the file: %s\n", strerror( errno ));
			return errno;
		}
	}
	off_t N = sb.st_size;
	off_t Q = 16; //text has to be at least 16 bytes?
	off_t K = N/Q; // TODO TEMP
	if(N < 2 * getpagesize()){
		Q = 1;
		K = N/Q;
	}

	// Structure to pass to the registration syscall
	struct sigaction new_action;
	memset(&new_action, 0, sizeof(new_action));
	// Assign pointer to our handler function
	new_action.sa_handler = my_signal_handler;
	// Setup the flags
	new_action.sa_flags = SA_SIGINFO;
	// Register the handler
	if( 0 != sigaction(SIGUSR1, &new_action, NULL) )
	{
		printf("Signal handle registration failed. %s\n", strerror(errno));
		return -1;
	}

	for(int i=0; i<Q; i++){
		pid_t cpid = fork();
		char* length = malloc(sizeof(length));
		char* offset = malloc(sizeof(off_t));
		if(i == 16){
			sprintf(length,"%ld",(long) N - (Q-1)* K);
		}
		else{
			sprintf(length,"%ld",(long)K);
		}
			sprintf(offset,"%ld",(long) i * K);
		if(cpid == 0) // child
		{
			char *argvv[] = {"Counter.c", argv[1], argv[2] , offset, length, NULL};
			execv("./coun",argvv);
			printf("execv failed: %s\n", strerror(errno));
			return -1;
		}
		else{
			int status;
			wait(&status);
			continue;
		}
	}

	printf("The amount of appearances of the character : %d\n", COUNT);


}
