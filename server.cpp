#include <windows.h>
#include <iostream>
#include <map>
#include <sstream>

#define CMD_SIZE 64

int main() {
    DWORD num_bytes;
    std::string pipe_name;
    std::cout << "Pipe name: ";
    std::cin >> pipe_name;
    std::string pipe_name2 = "\\\\.\\pipe\\" + pipe_name;
    HANDLE hPipe = CreateNamedPipe(pipe_name2.c_str(),
                                   PIPE_ACCESS_DUPLEX,
                                   PIPE_TYPE_MESSAGE|PIPE_READMODE_MESSAGE,
                                   PIPE_UNLIMITED_INSTANCES,
                                   64,
                                   64,
                                   0,
                                   NULL);
    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cout << "Error " << GetLastError() << " when creating pipe\n";
    }
    while (true) {
        std::cout << "Waiting for connection\n";
        if (!ConnectNamedPipe(hPipe, NULL)) {
            std::cout << "Error " << GetLastError() << " when connecting pipe\n";
        }
        std::map<std::string, std::string> data {};
        std::string command (CMD_SIZE, '\0');
        std::string keyword {}, key {}, value {}, response {};
        while (true) {
            std::cout << "Waiting for client command\n";
            if (!ReadFile(hPipe, &command[0], command.size(), &num_bytes, NULL)) {
                std::cout << "Error " << GetLastError() << " when reading command\n";
            }
            command.resize(command.find('\0'));
            std::istringstream parser {command};
            parser >> std::ws >> keyword;
            if (keyword == "set") {
                parser >> key >> value;
                data[key] = value;
                response = "acknowledged";
            }
            else if (keyword == "get") {
                parser >> key;
                if (data.find(key) != data.end()) {
                    response = "found " + data[key];
                }
                else {
                    response = "missing";
                }
            }
            else if (keyword == "list") {
                for (auto i = data.begin(); i != data.end(); ++ i) {
                    response += i->first + " ";
                }
            }
            else if (keyword == "delete") {
                parser >> key;
                auto del = data.find(key);
                if (del != data.end()) {
                    data.erase(del);
                    response = "deleted";
                }
                else {
                    response = "missing";
                }
            }
            else if (keyword == "quit") {
                if (!DisconnectNamedPipe(hPipe)) {
                    std::cout << "Error " << GetLastError() << " when disconnecting pipe\n";
                }
                command.replace(0, command.size(), command.size(), '\0');
                command.resize(CMD_SIZE, '\0');
                break;
            }
            else {
                std::cout << "Incorrect command\n";
                response = "incorrect command";
            }
            if (!WriteFile(hPipe, response.c_str(), response.size(), &num_bytes, NULL)) {
                std::cout << "Error " << GetLastError() << " when writing to pipe\n";
            }
            command.replace(0, command.size(), command.size(), '\0');
            command.resize(CMD_SIZE, '\0');
            response.clear();
            keyword.clear();
            key.clear();
            value.clear();
        }
        char y_or_n;
        bool to_exit = false;
        std::cout << "Do you want to destroy pipe \"" << pipe_name << "\" (y/n)?: ";
        while (std::cin >> y_or_n) {
            if (y_or_n == 'y') {
                if (!CloseHandle(hPipe)) {
                    std::cout << "Error " << GetLastError() << " when closing pipe\n";
                }
                to_exit = true;
                break;
            }
            else if (y_or_n == 'n') {
                break;
            }
            else {
                std::cout << "(y/n): ";
                continue;
            }
        }
        if (to_exit) {
            break;
        }
    }
    return 0;
}
