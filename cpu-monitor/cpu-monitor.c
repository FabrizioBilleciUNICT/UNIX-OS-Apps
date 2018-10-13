#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <fcntl.h>

#define NOS 30
#define SIZE 1024

#define T_RAW 1
#define T_DELTA 2

typedef struct {
	long mtype;
	int user;
	int system;
	int idle;
} raw;


typedef struct {
	long mtype;
	float user;
	float system;
} delta;


void Sampler(int msqid, int num){
	FILE *proc;
	char buffer[SIZE];
	raw mraw;
	char *s;
	
	while(1){
		sleep(1);

		if((proc = fopen("/proc/stat", "r")) == NULL){
			perror("/proc/stat");
			exit(1);
		}

		if(fgets(buffer, SIZE, proc)>0) {
			int count = 1;
			s = strtok(buffer, " ");
			s = strtok(NULL, " ");

			while(s != NULL){
				if(count == 1) 
					mraw.user = atoi(s);
				else if(count == 3) 
					mraw.system = atoi(s);
				else if(count == 4) 
					mraw.idle = atoi(s);

				s = strtok(NULL, " ");
				count++;
			}

			mraw.mtype = T_RAW;

			if(msgsnd(msqid, &mraw, sizeof(raw)-sizeof(long), 0) == -1){
				perror("msgsnd");
				exit(1);
			}
		}

		fclose(proc);

		num--;
		if(num == 0) break;
	}	

	exit(0);
}

void Analyzer(int msqid, int num){
	raw mraw;
	delta mdelta;
	float tuser = 0; 
	float tsystem = 0;
	float user = 0;
	float system = 0;


	while(1){
		if(msgrcv(msqid, &mraw, sizeof(raw)-sizeof(long), T_RAW, 0) == -1){
			perror("msgrcv");
			exit(1);
		}

		if(tuser != 0) {
			user = ((mraw.user - tuser)/tuser) * 100000;
			system = ((mraw.system - tsystem)/tsystem) * 100000;

			mdelta.user = user;
			mdelta.system = system;
			mdelta.mtype = T_DELTA;

			if(msgsnd(msqid, &mdelta, sizeof(delta)-sizeof(long), 0) == -1){
				perror("msgsnd");
				exit(1);
			}
		}

		tuser = mraw.user;
		tsystem = mraw.system;

		num--;
		if(num == 0) break;
	}

	exit(0);
} 

void Plotter(int msqid, int num){
	delta mdelta;
	int i;

	while(1){
		if(msgrcv(msqid, &mdelta, sizeof(delta)-sizeof(long), T_DELTA, 0) == -1){
			perror("msgrcv");
			exit(1);
		}

		int msystem = (mdelta.system / 100)*60;
		int muser = (mdelta.user / 100)*60;

		for(i=0; i<60; i++) {
			if(i <= msystem)
				printf("#");
			else if(i <= muser+msystem)
				printf("*");
			else
				printf("_");
		}

		printf(" s: %f%%", mdelta.system);
		printf(" u: %f%%\n", mdelta.user);

		num--;
		if(num == 0) break;
	}

	exit(0);
}


int main(int argc, char **argv){
	int msqid;
	int nos;

	if(argc > 2) {
		printf("Usage: %s [number of samples]\n", argv[0]);
		exit(1);
	}

	if(argc == 2)
		nos = atoi(argv[1]);
	else 
		nos = NOS;

	if((msqid = msgget(IPC_PRIVATE, IPC_CREAT|IPC_EXCL|0600)) == -1){
		perror("msgget");
		exit(1);
	}

	if(fork() == 0) Sampler(msqid, nos);
	else if(fork() == 0) Analyzer(msqid, nos);
	else if(fork() == 0) Plotter(msqid, nos-1);
	else {
		wait(NULL);
		wait(NULL);
		wait(NULL);

		msgctl(msqid, IPC_RMID, NULL);

		exit(0);
	}


	return 0;
}
