#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <string.h>
#include <sys/wait.h>

#define NAME_FILE 256
#define NUM_CHARS 26

#define T_MES 1

typedef struct {
	long mtype;
	int id;
	char pathname[NAME_FILE];
	int countings[NUM_CHARS];
} mmsg;

void process_T(int id, int msg_fd, char *pathname){
	FILE *f;
	long num_chars = 0;
	mmsg mymsg;

	if((f = fopen(pathname, "r")) == NULL){
		perror(pathname);
		exit(1);
	}

	char a = 'A';
	char z = 'Z';

	for(int i=0; i<NUM_CHARS; i++)
		mymsg.countings[i] = 0;

	char c;
	while((c = fgetc(f)) != EOF){
		for(int i=a; i<=z ; i++){
			if(toupper(c) == i) {
				mymsg.countings[i-a]++;
			}
		}
	}
	mymsg.mtype = T_MES;
	mymsg.id = id;
	strcpy(mymsg.pathname, pathname);

	if(msgsnd(msg_fd, &mymsg, sizeof(mmsg)-sizeof(long), 0) == -1){
		perror("msgsnd");
		exit(1);
	}

	printf("process T-%d on file '%s':\n    ", id, pathname);
	for(int i=0; i<NUM_CHARS; i++) 
		printf("%c:%d ", tolower(i+a), mymsg.countings[i]);
	printf("\n\n");

	exit(0);
}

int main(int argc, char **argv){
	int i, children = 0;
	int msg_fd;
	mmsg mymsg;
	char a = 'A';
	int tot_chars[NUM_CHARS];

	if(argc < 2){
		printf("Usage: %s  <file-1> <file-2> <...>\n", argv[0]);
		exit(1);
	}

	if((msg_fd = msgget(IPC_PRIVATE, IPC_CREAT|IPC_EXCL|0660)) == -1){
		perror("msgget");
		exit(1);
	}

	for(i=1; i<argc; i++){
		if(fork() == 0) {
			process_T(i, msg_fd, argv[i]);
		}
		children++;
	}

	for(i=0; i<children; i++)
		wait(NULL);

	for(i=0; i<NUM_CHARS; i++)
			tot_chars[i] = 0;

	do {
		if(msgrcv(msg_fd, &mymsg, sizeof(mmsg)-sizeof(long), T_MES, 0) == -1){
			perror("msgrcv");
			exit(1);
		}

		for(i=0; i<NUM_CHARS; i++)
			tot_chars[i] += mymsg.countings[i];

		children--;
	} while(children > 0);

	printf("process parent P:\n    ");
	for(i=0; i<NUM_CHARS; i++) 
		printf("%c:%d ", tolower(i+a), tot_chars[i]);

	printf("\n");

	int indexMax = 0;
	for(i=0; i<NUM_CHARS-1; i++) {
		if(tot_chars[i] > tot_chars[indexMax]) {
			indexMax = i;
		}
	}

	printf("    most used letter: '%c':%d\n", tolower(indexMax+a), tot_chars[indexMax]);


	
	msgctl(msg_fd, IPC_RMID, 0);

	return 0;
}
