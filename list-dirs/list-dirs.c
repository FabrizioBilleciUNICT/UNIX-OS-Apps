#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/wait.h>

#define SIZE 256

#define S_MUTEX 0
#define S_FULL 1
#define S_EMPTY 2

#define IS_F 0
#define IS_D 1

int WAIT(int sem_des, int num_sem){
	struct sembuf op[1] = { {num_sem, -1, 0} };
	return semop(sem_des, op, 1);
}

int SIGNAL(int sem_des, int num_sem){
	struct sembuf op[1] = { {num_sem, +1, 0} };
	return semop(sem_des, op, 1);
}

typedef struct {
	long size;
	char name[SIZE];
	int mtype;
} minfo;

void reader(char *pathname, int shmid, int semid){
	DIR *mdir;
	struct dirent *direntbuf;
	struct stat statbuf;
	minfo *s;

	if((mdir = opendir(pathname)) == NULL){
		perror(pathname);
		exit(1);
	}

	if((s = (minfo*)shmat(shmid, NULL, 0)) == (minfo*)-1){
		perror("r-shmat");
		exit(1);
	}

    chdir(pathname);

	while((direntbuf = readdir(mdir)) != NULL){
		stat(direntbuf->d_name, &statbuf);

		if(S_ISDIR(statbuf.st_mode)) {
			if((strcmp(direntbuf->d_name, ".") != 0) && (strcmp(direntbuf->d_name, "..") != 0)){
				WAIT(semid, S_EMPTY);
				WAIT(semid, S_MUTEX);

				strcpy(s->name, direntbuf->d_name);	
				s->mtype = IS_D;

				SIGNAL(semid, S_MUTEX);
				SIGNAL(semid, S_FULL);
			}
		} else {
			WAIT(semid, S_EMPTY);
			WAIT(semid, S_MUTEX);

			strcpy(s->name, direntbuf->d_name);
			s->size = statbuf.st_size;
			s->mtype = IS_F;

			SIGNAL(semid, S_MUTEX);
			SIGNAL(semid, S_FULL);
		}
	}

	closedir(mdir);
	exit(0);
}

void fileConsumer(int shmid, int semid){
	minfo *s;

	if((s = (minfo*)shmat(shmid, NULL, SHM_RDONLY)) == (minfo*)-1){
		perror("file-shmat");
		exit(1);
	}

	while(1) {
		WAIT(semid, S_FULL);
		WAIT(semid, S_MUTEX);
		
		if(s->mtype == IS_F) {
			printf("%s [file size %ld bytes]\n", s->name, s->size);
			SIGNAL(semid, S_MUTEX);
			SIGNAL(semid, S_EMPTY);
		} else {
			SIGNAL(semid, S_MUTEX);
			SIGNAL(semid, S_FULL);
		}
	}
	exit(0);
}

void dirConsumer(int shmid, int semid){
	minfo *s;

	if((s = (minfo*)shmat(shmid, NULL, SHM_RDONLY)) == (minfo*)-1){
		perror("dir-shmat");
		exit(1);
	}

	while(1) {
		WAIT(semid, S_FULL);
		WAIT(semid, S_MUTEX);

		if(s->mtype == IS_D) {
			printf("%s [directory]\n", s->name);
			SIGNAL(semid, S_MUTEX);
			SIGNAL(semid, S_EMPTY);
		} else {
			SIGNAL(semid, S_MUTEX);
			SIGNAL(semid, S_FULL);
		}
	}
	exit(0);
}

int main(int argc, char **argv){
	struct stat statbuf;
	int shmid, semid;
	int children=0;
	int dc,fc;

	if(argc < 2){
		printf("Usage: %s <dir1> <dir2> <...>\n", argv[0]);
		exit(1);
	}

	if((shmid = shmget(IPC_PRIVATE, sizeof(minfo), IPC_CREAT|IPC_EXCL|0600)) == -1){
		perror("shm");
		exit(1);
	}

	if((semid = semget(IPC_PRIVATE, 3, IPC_CREAT|IPC_EXCL|0600)) == -1){
		perror("sem");
		exit(1);
	}

	semctl(semid, S_MUTEX, SETVAL, 1);
	semctl(semid, S_EMPTY, SETVAL, 1);
	semctl(semid, S_FULL, SETVAL, 0);

	for(int i=1; i<argc; i++){
		stat(argv[i], &statbuf);
		if (S_ISDIR(statbuf.st_mode)) {
			children++;
			if(fork() == 0) 
				reader(argv[i], shmid, semid);
		}
	}

	if((fc = fork()) == 0){
		fileConsumer(shmid, semid);
	}
	if((dc = fork()) == 0){
		dirConsumer(shmid, semid);
	}

	for(int i=0; i<children; i++)
		wait(NULL);

	kill(dc, SIGKILL);
	kill(fc, SIGKILL);


	shmctl(shmid,IPC_RMID,NULL);
	semctl(semid, 0, IPC_RMID, 0);

	exit(0);

	return 0;
}
