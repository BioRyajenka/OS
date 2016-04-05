#include <unistd.h>
#include <string>
#include <sstream>
#include <iostream>
#include <thread>

int main()
{
    std::thread mess_with_stdin([] () {
            for (int i = 0; i < 10; ++i) {
                std::stringstream msg;
                msg << "Self-message #" << i
                    << ": Hello! How do you like that!?\n";
                auto s = msg.str();
                int kek = write(STDIN_FILENO, s.c_str(), s.size());
                std::cout << "kek " << kek << "\n";
            }
        });

    std::string str;
    while (getline(std::cin, str))
        std::cout << "String: " << str << std::endl;

    mess_with_stdin.join();
}