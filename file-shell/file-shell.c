#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/mman.h>

#define SIZE 1024

#define T_NUM 1
#define T_SIZE 2
#define T_SEARCH 3
#define T_END 4
#define T_PARENT 5

typedef struct {
	long mtype;
	int des;
	char charf[1];
	char file_name[SIZE];
} message;

typedef struct {
	long mtype;
	long n;
} message_r;

void child_process(int msqid, int id, char *pathdir){
	message mymsg;
	message_r mymsgr;
	DIR *mdir;
	struct dirent *mdirent;

	if((mdir = opendir(pathdir)) == NULL){
		perror(pathdir);
		exit(1);
	}

	chdir(pathdir);

	while(1){
		msgrcv(msqid, &mymsg, sizeof(message)-sizeof(long), id, 0);
		if(mymsg.mtype == T_END) break;

		int n = 0;
		int fd = 0;
		seekdir(mdir, 0);
		char file_name[SIZE];
		switch(mymsg.des){
			case T_NUM:
				while((mdirent = readdir(mdir)) != NULL) {
					if((strcmp(mdirent->d_name, ".") == 0) || (strcmp(mdirent->d_name, "..") == 0)) continue;
					struct stat statbuf;
					stat(mdirent->d_name, &statbuf);
					if (!S_ISDIR(statbuf.st_mode)) n++;
				}
			break;
			case T_SIZE:
				while((mdirent = readdir(mdir)) != NULL) {
					if((strcmp(mdirent->d_name, ".") == 0) || (strcmp(mdirent->d_name, "..") == 0)) continue;
					struct stat statbuf;
					stat(mdirent->d_name, &statbuf);
					if (!S_ISDIR(statbuf.st_mode)) n = n + statbuf.st_size;	
				}
			break;
			case T_SEARCH:
				while((mdirent = readdir(mdir)) != NULL) {
					if((strcmp(mdirent->d_name, ".") == 0) || (strcmp(mdirent->d_name, "..") == 0)) continue;
					if(strcmp(mdirent->d_name, mymsg.file_name) == 0){
						strcpy(file_name, mdirent->d_name);
						break;
					}
				}

				if((fd = open(file_name, O_RDONLY)) == -1){
					perror(file_name);
					break;
				}

				char *file;
				if((file = (char*)mmap(NULL, SIZE, PROT_READ, MAP_PRIVATE, fd, 0)) == (char*)-1){
					perror("mmap");
					break;
				}

				for(int i=0; i<strlen(file); i++){
					if(file[i] == mymsg.charf[0]) n++;
				}

				close(fd);
				munmap(file, SIZE);

			break;
		}

		mymsgr.mtype = T_PARENT;
		mymsgr.n = n;
		msgsnd(msqid, &mymsgr, sizeof(message_r)-sizeof(long), 0);
	}

	exit(0);
}

int main(int argc, char **argv){
	int msqid;
	int children = 0;
	message mymsg;
	message_r mymsgr;

	if(argc < 2){
		printf("Usage: %s <directory-1> <directory-2> <...>\n", argv[0]);
		exit(1);
	}

	if((msqid = msgget(IPC_PRIVATE, IPC_CREAT|IPC_EXCL|0660)) == -1){
		perror("msgget");
		exit(1);
	}

	for(int i=1; i<argc; i++){
		struct stat statbuf;
		stat(argv[i], &statbuf);
		if (S_ISDIR(statbuf.st_mode)) {
			if(fork() == 0){
				child_process(msqid, children+1, argv[i]);
			}
			children++;
		}
	}

	char command[SIZE];	
	do {
		printf("file shell> ");
		fgets(command, SIZE, stdin);

		if(strstr(command, "num-files ") != 0){
			int len = strlen("num-files ");
			strcpy(command, command+len);
			if(atoi(command) > children) {
				printf("Number too large. [%d] It is not present in the list.\n", atoi(command));
			} else {
				mymsg.mtype = atoi(command); 
				mymsg.des = T_NUM;
				msgsnd(msqid, &mymsg, sizeof(mymsg)-sizeof(long), 0);

				msgrcv(msqid, &mymsgr, sizeof(mymsgr)-sizeof(long), T_PARENT, 0);
				printf("    %ld file\n", mymsgr.n);
			}
		}

		if(strstr(command, "total-size ") != 0){
			int len = strlen("total-size ");
			strcpy(command, command+len);
			if(atoi(command) > children) {
				printf("Number too large. [%d] It is not present in the list.\n", atoi(command));
			} else {
				mymsg.mtype = atoi(command); 
				mymsg.des = T_SIZE;
				msgsnd(msqid, &mymsg, sizeof(mymsg)-sizeof(long), 0);

				msgrcv(msqid, &mymsgr, sizeof(mymsgr)-sizeof(long), T_PARENT, 0);
				printf("    %ld bytes\n", mymsgr.n);
			}
		}

		if(strstr(command, "search-char ") != 0){
			int len = strlen("search-char ");
			strcpy(command, command+len);

			char s[SIZE];
			strcpy(s, command);
			strtok(s, " ");

			char *file_name;
			file_name = strtok(NULL, " ");

			char *charf;
			charf = strtok(NULL, " ");
			if(charf == NULL) {
				printf("Insert a character to find.\n");
				continue;
			}

			if(atoi(command) > children) {
				printf("Number too large. [%d] It is not present in the list.\n", atoi(command));
			} else {
				mymsg.mtype = atoi(command); 
				mymsg.des = T_SEARCH;
				strcpy(mymsg.charf, charf);
				strcpy(mymsg.file_name, file_name);
				msgsnd(msqid, &mymsg, sizeof(mymsg)-sizeof(long), 0);

				msgrcv(msqid, &mymsgr, sizeof(mymsgr)-sizeof(long), T_PARENT, 0);
				printf("    %ld\n", mymsgr.n);
			}
		}
	} while(strcmp(command, "quit\n") != 0);

	for(int i=1; i<children; i++) {
		mymsg.mtype = i; 
		mymsg.des = T_END;			
		msgsnd(msqid, &mymsg, sizeof(mymsg)-sizeof(long), 0);
	}

	msgctl(msqid, IPC_RMID, NULL);

	return 0;
}
