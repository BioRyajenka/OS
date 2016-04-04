#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

void my_handler(int signal, siginfo_t* info, void* unused) {
	printf("SIGUSR%d from %d\n", signal == SIGUSR1 ? 1 : 2, info->si_pid);
	exit(0);
}

int main() {
	struct sigaction sigact;
	sigact.sa_flags = SA_SIGINFO;
	sigact.sa_sigaction = my_handler;

	sigset_t set;
	
	if (sigemptyset(&set) != 0 || sigaddset(&set, SIGUSR1) != 0 || sigaddset(&set, SIGUSR2) != 0) {
		fprintf(stderr, "Error changing sigaction mask\n");
	}

	sigact.sa_mask = set;

	if (sigaction(SIGUSR1, &sigact, NULL) != 0 || sigaction(SIGUSR2, &sigact, NULL) != 0) {
		fprintf(stderr, "Error setting my_handler instead of default\n");
	}
	
	sleep(10);
	
	printf("No signals were caught.\n");
	return 0;
}