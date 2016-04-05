#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h> 
#include <cstdlib>
#include <signal.h>
//#include <string.h>

#include <string>
#include <vector>
#include <sstream>
#include <iostream>

std::string command_buffer;
std::string summary_buffer;

bool read_next_command(std::string &s) {
	printf("$");
	fflush(stdout);

	const int BUFFER_SIZE = 1024;
	char buffer[BUFFER_SIZE];
	int received = 0;
	while ((received = read(STDIN_FILENO, buffer, BUFFER_SIZE)) > 0) {
		buffer[received] = 0;
		command_buffer += buffer;
		summary_buffer += buffer;

		std::size_t found = command_buffer.find("\n");
		if (found != std::string::npos) {
			s = command_buffer.substr(0, found);
			command_buffer = command_buffer.substr(found + 1, (int)command_buffer.length() - found - 1);



			//command_buffer = "";
			return true;
		}
	}
	if (received == -1) {
		perror("read");
	}

	// TODO: copypast
	std::size_t found = command_buffer.find("\n");
	if (found != std::string::npos) {
		s = command_buffer.substr(0, found);
		command_buffer = command_buffer.substr(found + 1, (int)command_buffer.length() - found - 1);
		return true;
	}

	return false;
}

std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> res;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
    	if (!item.empty()) res.push_back(item);
    }
    return res;
}

std::vector<std::string> fetch_subcommands(std::string s) {
	return split(s, '|');
}

void link_stream(int to_what, int what) {
	while ((dup2(to_what, what) == -1) && (errno == EINTR)) {}
}

void execute_command(std::string command) {
	std::vector<std::string> ss = split(command, ' ');
	char *argv[ss.size() + 1];
	for (size_t i = 0; i < ss.size(); i++) {
		argv[i] = const_cast<char*>(ss[i].c_str());
	}
	argv[ss.size()] = 0;

	// fprintf(stderr, "executing");
	// for (auto &s : ss) {
	// 	fprintf(stderr, " %s", s.c_str());
	// }
	//fprintf(stderr, "\n");

	execvp(argv[0], argv);
}

std::string name;
bool busy;

void signalHandler(int signum) {
	//std::cout << name << "\n";
	if (name == "child") {
		signal(signum, SIG_DFL);
		raise(signum);
		return;
	}
	
	if (!busy) {
		printf("\n");
		exit(signum);
	}
}

int main() {
	signal(SIGINT, signalHandler);  
	name = "parent";
	std::string command;
	busy = false;
	while (read_next_command(command)) {
		busy = true;
		//printf("command is \"%s\"\n", command.c_str());
		std::vector<std::string> subcommands = fetch_subcommands(command);
		std::vector<int*> pipes;
		for (size_t i = 0; i < subcommands.size(); i++) {
			pipes.push_back(new int[2]);
			if (pipe(pipes.back()) == -1) { //pipe2(pipes.back(), O_CLOEXEC) == -1) {
				perror("pipe");
				return 0;
			}
		}

//		int finished_childs = 0;

		for (size_t i = 0; i < subcommands.size(); i++) {
			//fprintf(stderr, "i = %d, command = %s\n", i, subcommands[i].c_str());
			// executing
			int pid;
			if ((pid = fork()) == 0) {
				// child
				name = "child";
				if (i > 0) {
					link_stream(pipes[i][0], STDIN_FILENO);
					close(pipes[i][0]);
				}
				if (i < subcommands.size() - 1) {
					link_stream(pipes[i + 1][1], STDOUT_FILENO);
					close(pipes[i + 1][1]);
				}

//				sleep(10);

				execute_command(subcommands[i]);
				fprintf(stderr, "command '%s' call wasn't successful\n", subcommands[i].c_str());
				return 0;
			}

			if (i > 0) {
				close(pipes[i][0]);
			}
			if (i < subcommands.size() - 1) {
				close(pipes[i + 1][1]);
			}
			// parent

			int status;
			while ((wait(&status)) > 0) {
		        if (status == 2) {
		        	printf("command %s was stopped\n", subcommands[i].c_str());
		        }
		    }
		}

		busy = false;
	}
	printf("\n");
	printf("summary buffer: %s\n", summary_buffer.c_str());
	return 0;
}
