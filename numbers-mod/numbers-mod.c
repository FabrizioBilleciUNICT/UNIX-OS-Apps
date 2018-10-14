#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <sys/wait.h>

#define N_X 0
#define N_Y 1
#define N_END 2
#define N_EMPTY 3

#define S_MUTEX 0
#define S_EMPTY 1
#define S_FULL_X 2
#define S_FULL_Y 3

#define SIZE 10

int WAIT(int sem_des, int num_sem){
	struct sembuf operazioni[1] = { {num_sem, -1, 0} };
	return semop(sem_des, operazioni, 1);
}

int SIGNAL(int sem_des, int num_sem){
	struct sembuf operazioni[1] = { {num_sem, +1, 0} };
	return semop(sem_des, operazioni, 1);
}

typedef struct {
	int mnumber;
	int mtype;
} data;


void process_Mod(data *mdata, int semqid, char *n){
	int count_end = 0;
	int i_n = atoi(n);
	int x = 0; 

	while(1){
		WAIT(semqid, S_FULL_X);
		WAIT(semqid, S_MUTEX);

		for(int i=0; i<SIZE; i++)
			if(mdata[i].mtype == N_END) 
				count_end++;
		
		if(count_end == SIZE) {
			SIGNAL(semqid, S_MUTEX);
			SIGNAL(semqid, S_FULL_Y);
			break;
		}

		for(int i=0; i<SIZE; i++){
			if(mdata[i].mtype == N_X){
				x = mdata[i].mnumber;
				x = (x % i_n);
				mdata[i].mnumber = x;
				mdata[i].mtype = N_Y;
			}
		}

		SIGNAL(semqid, S_MUTEX);
		SIGNAL(semqid, S_FULL_Y);
	}

	exit(0);

}

void process_Out(data *mdata, int semqid){
	int count_end;

	while(1){
		WAIT(semqid, S_FULL_Y);
		WAIT(semqid, S_MUTEX);

		for(int i=0; i<SIZE; i++)
			if(mdata[i].mtype == N_END) 
				count_end++;
		
		if(count_end == SIZE) {
			SIGNAL(semqid, S_MUTEX);
			SIGNAL(semqid, S_EMPTY);
			break;
		}

		for(int i=0; i<SIZE; i++){
			if(mdata[i].mtype == N_Y){
				printf("%d\n", mdata[i].mnumber);
				mdata[i].mtype = N_EMPTY;
			}
		}

		SIGNAL(semqid, S_MUTEX);
		SIGNAL(semqid, S_EMPTY);
	}

	exit(0);
}

void process_P(data *mdata, int semqid, char *input){
	int fd;
	FILE *f;

	if((fd = open(input, O_RDONLY)) == -1){
		perror(input);
		exit(1);
	}

	if((f = fdopen(fd, "r")) == NULL){
		perror(input);
		exit(1);
	}

	for(int i=0; i<SIZE; i++){
		mdata[i].mnumber = 0;
		mdata[i].mtype = N_EMPTY;
	}

	char s[100];
	while(fgets(s, sizeof(s), f)){
		int x = atoi(s);

		WAIT(semqid, S_EMPTY);
		WAIT(semqid, S_MUTEX);

		for(int i=0; i<SIZE; i++){
			if(mdata[i].mtype == N_EMPTY){
				mdata[i].mnumber = x;
				mdata[i].mtype = N_X;
				break;
			}
		}

		SIGNAL(semqid, S_MUTEX);
		SIGNAL(semqid, S_FULL_X);
	}

	WAIT(semqid, S_EMPTY);
	WAIT(semqid, S_MUTEX);

	for(int i=0; i<SIZE; i++)
		mdata[i].mtype = N_END;

	SIGNAL(semqid, S_MUTEX);
	SIGNAL(semqid, S_EMPTY);
}

int main(int argc, char **argv){
	int shmid, semqid;
	data *mdata;

	if(argc != 3) {
		printf("Usage: %s  <input file> <modulus number>\n", argv[0]);
		exit(1);
	}

	if((shmid = shmget(IPC_PRIVATE, sizeof(data)*10, IPC_CREAT|IPC_EXCL|0660)) == -1){
		perror("shmget");
		exit(1);
	}

	if((mdata = (data*)shmat(shmid, NULL, 0)) == (data*)-1){
		perror("shmat");
		exit(1);	
	}

	if((semqid = semget(IPC_PRIVATE, 4, IPC_CREAT|IPC_EXCL|0660)) == -1){
		perror("semget");
		exit(1);
	}

	semctl(semqid, S_MUTEX, SETVAL, 1);
	semctl(semqid, S_EMPTY, SETVAL, SIZE);
	semctl(semqid, S_FULL_X, SETVAL, 0);
	semctl(semqid, S_FULL_Y, SETVAL, 0);

	if(fork() == 0){
		process_Out(mdata, semqid);
	} 
	else if(fork() == 0){
		process_Mod(mdata, semqid, argv[2]);
	} else {
		process_P(mdata, semqid, argv[1]);

		wait(NULL);
		wait(NULL);

		shmctl(shmid, IPC_RMID, NULL);
		semctl(semqid, 0, IPC_RMID, NULL);

		exit(0);
	}	

	return 0;
}
