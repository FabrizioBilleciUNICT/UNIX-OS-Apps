#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

#define SIZE 1024

void reader(char *pathname, int id, int fildes[]){
	int fd;
	char *file_content;

	if((fd = open(pathname, O_RDONLY)) == -1){
		perror(pathname);
		exit(1);
	}

	if((file_content = (char*) mmap(NULL, SIZE, PROT_READ, MAP_PRIVATE, fd, 0)) == (char*)-1){
		perror("mmap");
		exit(1);
	}

	close(fildes[0]);
	write(fildes[1], file_content, SIZE);

	munmap(file_content, SIZE);

	exit(0);
}

void outputer(int fildes[], int nfiles){
	char out[SIZE];

	close(fildes[1]);

	int c = 0;
	while(1){
		c = read(fildes[0], out, SIZE);
		if(c > 0)
			printf("%s\n", out);
		else {
			nfiles--;
			if(nfiles == 0)
				break;
		}
	}
	
	exit(0);
}

int main(int argc, char **argv){
	struct stat statbuf;
	int children = 0;
	int fildes[2];
	int fildes_out[2];
	char file_content[SIZE];

	if(argc < 4){
		printf("Usage: %s [-v] [-i] [word] <file-1> [file-2] [file-3] [...]\n", argv[0]);
		exit(1);
	}

	if(pipe(fildes) == -1){
		perror("pipe");
		exit(1);
	}

	if(pipe(fildes_out) == -1){
		perror("pipe_out");
		exit(1);
	}

	int pid;
	if(fork()==0) 
		outputer(fildes_out, argc-3);
	else {
		for(int i=3; i<argc; i++){
			stat(argv[i], &statbuf);
			if (!S_ISDIR(statbuf.st_mode)){
				if((pid = fork()) == 0){
					reader(argv[i], children+1, fildes);
				}
				children++;
			} 
			waitpid(pid, NULL, 0); 

			read(fildes[0], file_content, SIZE);

			char *s;
			char copy[SIZE];
			strcpy(copy, file_content);
			s = strtok(copy, " \n");
			while(s != NULL){
				if(strcmp(argv[1], "-i") == 0){
					if(strstr(s, argv[2]) != NULL) {
						char m[SIZE];
						char n[SIZE];
						strcpy(m, s);
						strcpy(n, argv[i]);
						strcat(n, ":");
						strcat(n, m);
						write(fildes_out[1], n, SIZE);
					}
				} else if(strcmp(argv[1], "-v") == 0) {
					if(strcasestr(s, argv[2]) == NULL) {
						char m[SIZE];
						char n[SIZE];
						strcpy(m, s);
						strcpy(n, argv[i]);
						strcpy(n, ":");
						strcat(n, m);
						write(fildes_out[1], n, SIZE);
					}
				}
				s = strtok(NULL, " \n");
			}
		}	
	}

	exit(0);
	
	return 0;
}
