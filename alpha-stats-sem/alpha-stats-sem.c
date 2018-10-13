#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define NUMLET 26
#define SIZE 2048

#define S_FULL 0
#define S_EMPTY 1
#define S_MUTEX 2

typedef struct {
	int alphabet[NUMLET];
} data;


int WAIT(int sem_des, int num_semaforo){
	struct sembuf operazioni[1] = {{num_semaforo,-1,0}};
	return semop(sem_des, operazioni, 1);
}
int SIGNAL(int sem_des, int num_semaforo){
	struct sembuf operazioni[1] = {{num_semaforo,+1,0}};
	return semop(sem_des, operazioni, 1);
}


void child(int shmid, int semid, char **argv, int index){
	int fd;
	FILE *f;
	data *mdata;
	char *content;
	char *filename;
	filename = argv[index];
	struct stat statbuf;

	if((fd = open(filename, O_RDONLY)) == -1){
		perror(filename);
		exit(1);
	}

	if(fstat(fd, &statbuf) == -1){
		perror("fstat");
		exit(1);
	}

	if((f = fdopen(fd, "r")) == NULL){
		perror("fdopen");
		exit(1);
	}

	if((content = (char*)mmap(NULL, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0)) == (char*)-1){
		perror("mmap");
		exit(1);
	}

	if((mdata = (data*)shmat(shmid, NULL, 0)) == (data*)-1){
		perror("shmat");
		exit(1);
	}

	char c;
	int a = 'A';
	int z = 'z';
	while((c = fgetc(f)) != EOF){
		//WAIT(semid, S_EMPTY);
		WAIT(semid, S_MUTEX);

		usleep(rand() % 40);

		int cx = toupper(c);

		if(cx >= a && cx <= z) {
			mdata->alphabet[cx-a] += 1;
		}

		SIGNAL(semid, S_MUTEX);
		//SIGNAL(semid, S_FULL);
	}

	munmap(content, statbuf.st_size);
	fclose(f);
	close(fd);

	exit(0);
}

int main(int argc, char **argv){
	int shmid, semid;
	data *mdata;

	if(argc < 2){
		printf("Usage: %s <file-1> [file-2] [file-3] [...]\n", argv[0]);
		exit(1);
	}

	if((shmid = shmget(IPC_PRIVATE, sizeof(data), IPC_CREAT|IPC_EXCL|0660)) == -1){
		perror("shmget");
		exit(1);
	}

	if((mdata = (data*)shmat(shmid, NULL, 0)) == (data*)-1){
		perror("shmat");
		exit(1);
	}

	if((semid = semget(IPC_PRIVATE, 3, IPC_CREAT|IPC_EXCL|0660)) == -1){
		perror("semget");
		exit(1);
	}

	semctl(semid, S_MUTEX, SETVAL, 1);
	semctl(semid, S_FULL, SETVAL, 0);
	semctl(semid, S_EMPTY, SETVAL, 1);

	for(int i=0; i<NUMLET; i++)
		mdata->alphabet[i] = 0;


	for(int i=1; i<argc; i++){
		if(fork() == 0){
			child(shmid, semid, argv, i);
		}
	}

	for(int i=1; i<argc; i++)
		wait(NULL);

	int tot = 0;
	for(int i=0; i<NUMLET; i++) {
		tot += mdata->alphabet[i];
	}

	int a = 'a';
	printf("frequence:\n");
	for(int i=0; i<NUMLET; i++){
		float f = ((float)mdata->alphabet[i]/(float)tot)*100.0;
		printf("%c: %f%%\n", (char)(a+i), f);
	}

	semctl(semid, 0, IPC_RMID, 0);
	shmctl(shmid, IPC_RMID, NULL);
	

	return 0;
}
