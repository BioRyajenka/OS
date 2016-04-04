#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

const unsigned int BUFFER_SIZE = 1024;
const unsigned int STANDART_INPUT = 0;
const unsigned int STANDART_OUTPUT = 1;

void cat(int input_fd, int output_fd) {
	const int BUFFER_SIZE = 1024;

	char buffer[BUFFER_SIZE];
	int received = 0;

	while ((received = read(input_fd, buffer, BUFFER_SIZE)) > 0) {
		int sent = 0;
		while (sent != received) {
			int currentlySent = write(output_fd, buffer + sent, received - sent);
			sent += currentlySent;
			if (currentlySent < 0) {
				fprintf(stderr, "Error writing to output\n");
				return;
			}
		}
	}

	if (received < 0) {
		fprintf(stderr, "Error reading from input\n");
	}
}

int main(int argc, char **argv) {
	cat(STANDART_INPUT, STANDART_OUTPUT);
	return 0;
}