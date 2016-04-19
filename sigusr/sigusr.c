#include <signal.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>


void my_handler(int signal, siginfo_t* info, void* unused) {
	printf("signal %d from %d\n", signal, info->si_pid);
	exit(0);
}

int main() {
	printf("main thread pid %d\n", (int)getpid());
	struct sigaction sigact;

	sigact.sa_flags = SA_SIGINFO;
	sigact.sa_sigaction = my_handler;

	sigset_t set;
	
	if (sigemptyset(&set) != 0) {
		fprintf(stderr, "Error emptying sigaction mask\n");
		return 0;
	}

	int i;

	for (i = 1; i <= 30; i++) {
		if (i != SIGKILL && i != SIGSTOP && sigaddset(&set, i) != 0) {
			fprintf(stderr, "Error changing sigaction mask\n");
			return 0;
		}
	}

	sigact.sa_mask = set;

	for (i = 1; i <= 30; i++) {
		if (i != SIGKILL && i != SIGSTOP && sigaction(i, &sigact, NULL) != 0) {
			fprintf(stderr, "Error setting my_handler instead of default\n");
			return 0;
		}
	}
	
	sleep(10);
	
	printf("No signals were caught.\n");
	return 0;
}