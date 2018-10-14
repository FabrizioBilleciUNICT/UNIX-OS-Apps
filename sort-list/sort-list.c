#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <wait.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define SIZE 1024

#define STR_CMP 1
#define INT_CMP 2

typedef struct {
	long mtype;
	char a[SIZE];
	char b[SIZE];
} strmsg;

typedef struct {
	long mtype;
	int compared;
} intmsg;

void sorter(int fildes[], int msqid){
	FILE *f;
	char riga[SIZE];
	int fd = fildes[0];
	int N = 0;
	strmsg mymsg;
	intmsg mimsg;

	if((f = fdopen(fd, "w+")) == NULL){
		perror("file");
		exit(1);
	}

	while(fgets(riga, SIZE, f) != NULL)
		N++;

	rewind(f);

	char vecc[N][SIZE];

	for(int i=0; i<N; i++){
		fgets(riga, SIZE, f);
		strcpy(vecc[i], riga);
	}

	for(int i=0; i<N-1; i++){
		for(int j=i+1; j<N; j++){
			strcpy(mymsg.a, vecc[i]);
			strcpy(mymsg.b, vecc[j]);
			mymsg.mtype = STR_CMP;

			msgsnd(msqid, &mymsg, sizeof(strmsg)-sizeof(long), 0);
			
			msgrcv(msqid, &mimsg, sizeof(intmsg)-sizeof(long), INT_CMP, 0);
			
			if(mimsg.compared > 0) {
				char tmp[SIZE];
				strcpy(tmp, vecc[i]);
				strcpy(vecc[i], vecc[j]);
				strcpy(vecc[j], tmp);
			}
		}
	}

	strcpy(mymsg.a, "");
	mymsg.mtype = STR_CMP;
	msgsnd(msqid, &mymsg, sizeof(strmsg)-sizeof(long), 0);

	for(int i=0; i<N; i++){
		printf("%s", vecc[i]);
	} 


	exit(0);
}

void comparer(int msqid){
	while(1) {
		strmsg mymsg;
		intmsg mimsg;

		if (msgrcv(msqid, &mymsg, sizeof(strmsg)-sizeof(long), STR_CMP, 0) == -1) {
	        perror("msgrcv");
	      	exit(1);
	    } 

	    if(strcmp(mymsg.a, "") == 0) break;

		mimsg.compared = strcmp(mymsg.a, mymsg.b);
		mimsg.mtype = INT_CMP;

		if (msgsnd(msqid, &mimsg, sizeof(intmsg)-sizeof(long), 0) == -1) {
	        perror("msgsnd");
	      	exit(1);
	    }
	}

	exit(0);
}

int main(int argc, char **argv){
	int fildes[2];
	int msqid;

	if(argc != 2){
		printf("Usage: %s file.txt\n", argv[0]);
		exit(1);
	}

	if(pipe(fildes) != 0){
		perror("pipe");
		exit(1);
	}

	if((fildes[0] = open(argv[1], O_RDWR)) == -1){
		perror(argv[1]);
		exit(1);
	}

	if((msqid = msgget(IPC_PRIVATE, IPC_CREAT|IPC_EXCL|0660)) == -1){
		perror("msg");
		exit(1);
	}

	if (fork() != 0) {
        if (fork() != 0) {
            wait(NULL);
            wait(NULL);
        } else {
            sorter(fildes, msqid);
        }
    } else {
        comparer(msqid);
    }
  
	msgctl(msqid, IPC_RMID, NULL);
	
	exit(0);

	return 0;
}
