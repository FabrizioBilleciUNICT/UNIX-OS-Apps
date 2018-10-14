#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#define SIZE 1024

#define S_SCANNER 0
#define S_STATER 1
#define S_PARENT 2

typedef struct {
	char pathname[SIZE];
	long size;
	int index;
	int n;
} mfile;

int WAIT(int sem_des, int num_semaforo){
	struct sembuf operazioni[1] = {{num_semaforo,-1,0}};
	return semop(sem_des, operazioni, 1);
}

int SIGNAL(int sem_des, int num_semaforo){
	struct sembuf operazioni[1] = {{num_semaforo,+1,0}};
	return semop(sem_des, operazioni, 1);
}


void scanner2(mfile *mymfile, int semid, char *pathname, int index){
	DIR *mdir;
	struct dirent *mdirent;
	struct stat statbuf;

	if((mdir = opendir(pathname)) == NULL){
		perror(pathname);
		exit(1);
	}

	if (chdir(pathname) == -1) {
        perror(pathname);
        exit(1);
    }

	while((mdirent = readdir(mdir)) != NULL){
		stat(mdirent->d_name, &statbuf);
		if(strcmp(mdirent->d_name, ".") != 0 && strcmp(mdirent->d_name, "..") != 0) {
			
			if (S_ISDIR(statbuf.st_mode)){
				scanner2(mymfile, semid, mdirent->d_name, index);
			} else {
				WAIT(semid, S_SCANNER);
				mymfile->index = index;
				mymfile->size = -1;
				strcpy(mymfile->pathname, mdirent->d_name);
				SIGNAL(semid, S_STATER);
			}	
		}
	}

	chdir("..");
	closedir(mdir);

}

void scanner(mfile *mymfile, int semid, char *pathname, int index){
	DIR *mdir;
	struct dirent *mdirent;
	struct stat statbuf;

	if((mdir = opendir(pathname)) == NULL){
		perror(pathname);
		exit(1);
	}

	if (chdir(pathname) == -1) {
        perror(pathname);
        exit(1);
    }

	while((mdirent = readdir(mdir)) != NULL){
		stat(mdirent->d_name, &statbuf);
		if(strcmp(mdirent->d_name, ".") != 0 && strcmp(mdirent->d_name, "..") != 0) {
			
			if (S_ISDIR(statbuf.st_mode)){
				scanner2(mymfile, semid, mdirent->d_name, index);
			} else {
				WAIT(semid, S_SCANNER);
				mymfile->index = index;
				mymfile->size = -1;
				strcpy(mymfile->pathname, mdirent->d_name);
				SIGNAL(semid, S_STATER);
			}	
		}
	}

	WAIT(semid, S_SCANNER);
	mymfile->index = index;
	mymfile->size = -1;
	strcpy(mymfile->pathname, "-1");
	mymfile->n += -1;
	SIGNAL(semid, S_STATER);

	chdir("..");
	closedir(mdir);

	exit(0);
}


void stater(int shmid, int semid){
	mfile *mymfile;
	struct stat statbuf;
	int n = 1;

	if((mymfile = (mfile*)shmat(shmid, NULL, 0)) == (mfile*)-1){
		perror("shmat");
		exit(1);
	}

	do {
		WAIT(semid, S_STATER);
		n = mymfile->n;
		stat(mymfile->pathname, &statbuf);
		mymfile->size = statbuf.st_blocks;
		SIGNAL(semid, S_PARENT);
	}
	while(n > 0);

	exit(0);
}

void parent(int shmid, int semid, int paths, char **argv){
	mfile *mymfile;
	long path_size[paths];
	char path_name[SIZE][paths];
	int n = 1;

	for(int i=0; i<paths; i++) path_size[i] = 0;

	if((mymfile = (mfile*)shmat(shmid, NULL, 0)) == (mfile*)-1){
		perror("shmat");
		exit(1);
	}

	do {
		WAIT(semid, S_PARENT);
		
		n = mymfile->n;
		if(n < 1) break;
		path_size[mymfile->index] += mymfile->size;

		SIGNAL(semid, S_SCANNER);
	}
	while(n > 0);

	for(int i=0; i<paths; i++){
		printf("%ld \t %s\n", path_size[i], argv[i+1]);
	}

}

int main(int argc, char **argv){
	int shmid, semid;
	mfile *mymfile;

	if(argc < 2){
		printf("%s [path-1] [path-2] […]\n", argv[0]);
		exit(1);
	}

	if((shmid = shmget(IPC_PRIVATE, sizeof(mfile), IPC_CREAT|IPC_EXCL|0660)) == -1){
		perror("shmget");
		exit(1);
	}

	if((semid = semget(IPC_PRIVATE, 3, IPC_CREAT|IPC_EXCL|0660)) == -1){
		perror("semget");
		exit(1);
	}

	semctl(semid, S_SCANNER, SETVAL, 1);
	semctl(semid, S_STATER, SETVAL, 0);
	semctl(semid, S_PARENT, SETVAL, 0);

	if((mymfile = (mfile*)shmat(shmid, NULL, 0)) == (mfile*)-1){
		perror("shmat");
		exit(1);
	}

	mymfile->n = argc-1; //num directories

	if(fork() == 0){
		stater(shmid, semid);
	} else {
		for(int i=1; i<argc; i++){
			if(fork() == 0){
				scanner(mymfile, semid, argv[i], i-1);
			}
		}

		parent(shmid, semid, argc-1, argv);

		for(int i=0; i<argc; i++) 
			wait(NULL);

		shmctl(shmid, IPC_RMID, NULL);
		semctl(semid, 0, IPC_RMID, 0);
	}


	return 0;
}
