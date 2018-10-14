#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <ctype.h>

#define SIZE 1000

typedef enum {
	UPPER,
	LOWER,
	CHANGE
} editor;

typedef enum{
	FILTER,
	END
} type;

typedef struct{
	long des;
	char mtext[SIZE];
	int mtype;
} message;


void filter(int msqid, int id, char *word, int mod){
	message messaggio;
	
	while(1) {
		msgrcv(msqid, &messaggio, sizeof(message)-sizeof(long), id, 0);
		messaggio.des = id+1;

		if(messaggio.mtype == END) break;

		char *pch;
		pch = strstr(messaggio.mtext, word);

		char *s1, *s2;

		if(mod == CHANGE) {
			char copy_word[SIZE];
			strcpy(copy_word, word);

			s1 = strtok(copy_word, ",");
			s2 = strtok(NULL, ",");
			pch = strstr(messaggio.mtext, s1);
		}
		
		switch(mod){
			case UPPER:
				if(pch != NULL) {
					for(int i=0; i<strlen(word); i++) 
						pch[i] = toupper(pch[i]);
				}
			break;
			case LOWER:
				if(pch != NULL) {
					for(int i=0; i<strlen(word); i++) 
						pch[i] = tolower(pch[i]);
				}
			break;
			case CHANGE:
				if(pch != NULL) {
					for(int i=0; i<strlen(s2); i++) 
						pch[i] = s2[i];
				}
			break;
		}

		msgsnd(msqid, &messaggio, sizeof(message)-sizeof(long), 0);
	}
	messaggio.mtype = END;
	msgsnd(msqid, &messaggio, sizeof(message)-sizeof(long), 0);
	exit(0);
}

int main(int argc, char **argv){
	FILE *file;
	char riga[SIZE];
	message messaggio;
	int msqid;
	int child_id = 0;

	if(argc <= 2) {
		printf("Usage: %s <file.txt> <filter-1> [filter-2] [...]", argv[0]);
		exit(1);
	}

	if((file = fopen(argv[1], "r")) == NULL){
		perror(argv[1]);
		exit(1);
	}

	if((msqid = msgget(IPC_PRIVATE, IPC_CREAT|0600)) == -1){
		perror("msgget");
		exit(1);
	}

	for(int i=2; i<argc; i++){
		switch(argv[i][0]){
			case '^':
				strcpy(argv[i],argv[i]+1);
				if(fork()==0){
					filter(msqid, child_id+1, argv[i], UPPER);
				}
				child_id++;
			break;
			case '_':
				strcpy(argv[i],argv[i]+1);
				if(fork()==0){
					filter(msqid, child_id+1, argv[i], LOWER);
				}
				child_id++;
			break;
			case '=':
				strcpy(argv[i],argv[i]+1);
				char copy[SIZE];
				strcpy(copy, argv[i]);
				char *s1, *s2;
				s1 = strtok(copy, ",");
				s2 = strtok(NULL, ",");

				if(strlen(s1) != strlen(s2))
					perror("Error! Strings have different length.");
				else {
					if(fork()==0){
						filter(msqid, child_id+1, argv[i], CHANGE);
					}
					child_id++;
				}
			break;
			
			default:
				printf("%s is not valid\n",argv[i]);
			break;
		}
	}

	
	while(fgets(riga, SIZE, file) != NULL){
		messaggio.mtype = FILTER;
		messaggio.des = 1;
		strcpy(messaggio.mtext, riga);
		
		msgsnd(msqid, &messaggio, sizeof(message)-sizeof(long), 0);
		msgrcv(msqid, &messaggio, sizeof(message)-sizeof(long), child_id+1, 0);
		printf("%s", messaggio.mtext);
	}
	
	fclose(file);
	msgctl(msqid, IPC_RMID, NULL);
	exit(0);
	
	return 0;
}
