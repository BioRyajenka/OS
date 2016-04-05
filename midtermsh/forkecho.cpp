#include <errno.h>
#include <unistd.h>
//#include <sys/wait.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h> 
#include <cstdlib>
#include <signal.h>
//#include <string.h>

int main() {
	pid_t pid;
	int pipefd[2];
	FILE* output;

	if (pipe(pipefd) == -1) {
		perror("pipe");
		return 0;
	}

//fcntl(pipefd[0], F_SETFL, O_NONBLOCK);
//fcntl(pipefd[1], F_SETFL, O_NONBLOCK);


	if ((pid = fork()) == -1) {
		perror("fork");
		return 0;
	}
	if (pid == 0) { // child
		printf("created child\n");
		//we need loop because dup2 can be interrupted by a signal
		while ((dup2(pipefd[1], STDOUT_FILENO ) == -1) && (errno == EINTR)) {}
		
		
		//why can we close immidiately?

		char *argv[] = { "ls", 0 };
    	execvp(argv[0], argv);


		fprintf(stderr, "execlp call wasn't successful");

		close(pipefd[0]);
		close(pipefd[1]);

		exit(1);
	}
	// The code below will be executed only by parent. You can write and read
	// from the child using pipefd descriptors, and you can send signals to 
	// the process using its pid by kill() function. If the child process will
	// exit unexpectedly, the parent process will obtain SIGCHLD signal that
	// can be handled (e.g. you can respawn the child process).

	// Now, you can write to the process using pipefd[0], and read from pipefd[1]:
	printf("created parent proc\n");

	//write(pipefd[0], "123", 3); // write message to the process
	close(pipefd[1]);

	char buf[128];
	ssize_t readed;
	printf("here1");

	while ((readed = read(pipefd[0], buf, sizeof(buf))) != 0) {
		printf("here2");
		if (readed == -1) {
			if (errno == EINTR) { // child wasn't created yet
				perror("errno");
				continue;
			} else {
				perror("read");
				return 0;
				//continue;
			}
		}
		buf[readed] = '\0';
		printf("child say %s\n", buf);
	}
	printf("here3");

	close(pipefd[0]);

	wait();

	//kill(pid, SIGKILL); //send signo signal to the child process
	return 0;
}