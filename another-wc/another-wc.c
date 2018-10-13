#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <ctype.h>

#define SIZE 1024

#define S_READER 0
#define S_WORKER 1

typedef struct {
	char c;
	char type;
} data;

int WAIT(int sem_des, int num_semaforo){
	struct sembuf operazioni[1] = {{num_semaforo,-1,0}};
	return semop(sem_des, operazioni, 1);
}

int SIGNAL(int sem_des, int num_semaforo){
	struct sembuf operazioni[1] = {{num_semaforo,+1,0}};
	return semop(sem_des, operazioni, 1);
}

void child(char *pathname, int shmid, int semid){
	char buffer[SIZE];
	buffer[0] = '\0';
	FILE *f;
	data *mdata;
	char mem[SIZE];
	mem[0] = '\0';

	if((mdata = (data*) shmat(shmid, NULL, 0)) == (data*)-1){
		perror("shmat");
		exit(1);
	}


	if(strcmp(pathname, "stdin") == 0){
		do {
			strcat(mem, buffer);
			printf(">");
			fgets(buffer, SIZE, stdin);

		} while(strcmp(buffer, "q\n") != 0);
		f = fopen("stdinn", "w+");
		fputs(mem, f);
		fclose(f);
		strcpy(pathname, "stdinn");
	} 

	if((f = fopen(pathname, "r")) == NULL){
		perror(pathname);
		exit(1);
	}

	mdata->type = 1;

	char c;
	while((c = fgetc(f)) != EOF){
		WAIT(semid, S_READER);
		mdata->c = c;
		SIGNAL(semid, S_WORKER);
	}

	WAIT(semid, S_READER);
	mdata->type = 0;
	SIGNAL(semid, S_WORKER);

	fclose(f);

	if(strcmp(pathname, "stdinn")==0)
		unlink(pathname);

	exit(0);
}

int main(int argc, char **argv){
	int shmid, semid;
	char pathname[256];
	data *mdata;

	if(argc > 2){
		printf("Usage: %s [text file]\n", argv[0]);
		exit(1);
	}

	if((shmid = shmget(IPC_PRIVATE, sizeof(data), IPC_CREAT|IPC_EXCL|0660)) == -1){
		perror("shmget");
		exit(1);
	}

	if((mdata = (data*) shmat(shmid, NULL, 0)) == (data*)-1){
		perror("shmat");
		exit(1);
	}

	if((semid = semget(IPC_PRIVATE, 2, IPC_CREAT|IPC_EXCL|0660)) == -1){
		perror("semget");
		exit(1);
	}

	semctl(semid, S_READER, SETVAL, 1);
	semctl(semid, S_WORKER, SETVAL, 0);


	if(argc == 1)
		strcpy(pathname, "stdin"); 
	else 
		strcpy(pathname, argv[1]);

	if(fork() == 0)
		child(pathname, shmid, semid);
	//else wait(NULL);

	else {
		long nchars = 0;
		long nwords = 0;
		long nrows = 0;

		while(1){
			WAIT(semid, S_WORKER);
			
			if(mdata->type == 0) break;

			if(isspace(mdata->c))
				nwords++;
			if(mdata->c == '\n')
				nrows++;

			nchars++;

			SIGNAL(semid, S_READER);
		}

		printf("\t%ld\t%ld\t%ld\t%s\n", nrows, nwords, nchars, pathname);

		shmctl(shmid, IPC_RMID, NULL);
		semctl(semid, 0, IPC_RMID, 0);

		exit(0);
	}
	
	return 0;
}
