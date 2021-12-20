#include <windows.h>
#include <iostream>
#include <map>
#include <sstream>

#define CMD_SIZE 64

using namespace std;

int main() {
    DWORD num_bytes;
    string command (64, '\0');
    string response (64, '\0');
    string pipe_name;
    cout << "Pipe name: ";
    cin >> pipe_name;
    string pipe_name2 = "\\\\.\\pipe\\" + pipe_name;
    HANDLE hPipe = CreateFile(pipe_name2.c_str(),
                              GENERIC_READ|GENERIC_WRITE,
                              0,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);
    if (hPipe == INVALID_HANDLE_VALUE) {
        cout << "Error " << GetLastError() << " connecting pipe\n";
    }
    while (true) {
        cout << "> ";
        getline(cin >> ws, command);
        if (!WriteFile(hPipe, command.c_str(), command.size(), &num_bytes, NULL)) {
            cout << "Error " << GetLastError() << " when writing to pipe\n";
        }
        if (command == "quit") {
            if (!CloseHandle(hPipe)) {
                cout << "Error " << GetLastError() << " when closing pipe\n";
            }
            break;
        }
        else {
            if (!ReadFile(hPipe, &response[0], response.size(), &num_bytes, NULL)) {
                cout << "Error " << GetLastError() << " when reading response\n";
            }
            else {
                cout << response << endl;
            }
        }
        command.replace(0, command.size(), command.size(), '\0');
        response.replace(0, response.size(), response.size(), '\0');
    }
    return 0;
}
