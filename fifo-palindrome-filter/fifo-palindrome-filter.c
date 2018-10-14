#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SIZE 2048

void process_r(char *pathname, int fd_rp){
	char *file;
	int fd;

	if(strcmp(pathname, "stdin")==0){
		char s[SIZE];
		char f[SIZE];
		do {
			printf("Insert a word> ");
			fgets(s, SIZE, stdin);
			strcat(f, s);
		} while(strcmp(s, "q\n") != 0);
		write(fd_rp, f, SIZE);
		exit(0);
	} else {
		if((fd = open(pathname, O_RDONLY)) == -1){
			perror(pathname);
			exit(1);
		}

		if((file = (char*)mmap(NULL, SIZE, PROT_READ, MAP_PRIVATE, fd, 0)) == (char*)-1){
			perror("mmap");
			exit(1);
		}

		write(fd_rp, file, SIZE);
	}


	exit(0);
}


void process_w(int fd){
	char s[SIZE];

	while(read(fd, s, SIZE) != EOF) {
		printf("%s\n", s);
	}
	
	exit(0);
}

int main(int argc, char **argv){
	const char *rpfifo = "rpfifo";
	const char *pwfifo = "pwfifo";
	int fd_rp, fd_pw;
	int idw;
	char arg_file[SIZE];
	char argn[SIZE];
	
	if(argc == 1){
		strcpy(argn, "stdin");
	} else if(argc == 2) {
		strcpy(argn, argv[1]);
	} else {
		printf("Usage: %s <file> (optional)\n", argv[0]);
		exit(1);
	}

	if(mkfifo(rpfifo, IPC_CREAT|IPC_EXCL|0660) == -1){
		perror(rpfifo);
		exit(1);
	}

	if(mkfifo(pwfifo, IPC_CREAT|IPC_EXCL|0660) == -1){
		perror(pwfifo);
		exit(1);
	}

	if((fd_rp = open(rpfifo, O_RDWR)) == -1){
		perror(rpfifo);
		exit(1);
	}

	if((fd_pw = open(pwfifo, O_RDWR)) == -1){
		perror(pwfifo);
		exit(1);
	}



	if(fork() == 0) 
		process_r(argn, fd_rp);
	else {
		wait(NULL);

		if((idw = fork()) == 0)
			process_w(fd_pw); 
		else {
			char s[SIZE];
			read(fd_rp, s, SIZE);

			char *k;
			k = strtok(s, "\n");
			int i=0;
			while(k != NULL) {
				int g = strlen(k);
				int isPal = 1;
				char a, b;
				for(int j=0; j<g/2; j++){
					a = toupper(k[j]);
					b = toupper(k[g-j-1]);

					if(a != b) 
						isPal = 0;
				}
				if(isPal) {
					write(fd_pw, k, SIZE);
				}
				k = strtok(NULL, "\n");
				i++;
			}

			unlink(rpfifo);
			unlink(pwfifo);

			exit(0);
		}
	}


	return 0;
}
