//
// Created by Amore (ALC) on 13.12.2025.
// Update by Amore (ALC) on 20.12.2025
//
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <map>
#include <algorithm>
#include <vector>
#include <functional>
#include <windows.h>

#include "modules/network.h"

using namespace std;

class AmoreHundEvelis {
private:
    string nameUser;
    vector<string> command;
    vector<string> args;
    string cmd;
    bool isStarted = true;
    // История команд
    map<int, string> historyCommands_and_id;
    int history_last_id = 0;
public:
    AmoreHundEvelis() {
        char* _nameUser = getenv("USERNAME");
        nameUser =  _nameUser ? _nameUser : "localuser";
        print_welcome_text();
        while_event_handler();
    }

    vector<string> split(string str, char delimiter) {
        vector<string> parts;
        size_t start = 0;
        size_t end = str.find(delimiter);

        while (end != string::npos) {
            parts.push_back(str.substr(start, end - start));
            start = end + 1;
            end = str.find(delimiter, start);
        }
        parts.push_back(str.substr(start));
        return parts;
    }

    void print_welcome_text() {
        cls();
        cout << "\033[1;32mAmore \033[1;34mHundEvelis\033[0m\033[1;31mExtreme\033[0m | \033[1;34mWELCOME BACK\033[0m\n\n" << endl << endl;
    }

    void cls() {
        system("cls");
    }

    void print_history() {
        for (const auto& command : historyCommands_and_id) {
            cout << "ID: " << command.first << " - " << "'" << command.second << "'" << endl;
        }
    }

    void portchecker(vector<string> args) {
        NetworkTools network(args);
    }

    void while_event_handler() {
        while (isStarted) {
            try {
                cout << "\033[34m" << nameUser << ":\033[0m \033[32m$\033[0m\033[34m~\033[0m ";
                string input;
                getline(cin, input);
                if (input.empty()) continue;
                if (input != "history") {
                    historyCommands_and_id[history_last_id] = input;
                    history_last_id++;
                }
                command = split(input, ' ');
                bool command_check = false;
                for (const string& arg : command) {
                    if (!command_check) {
                        command_check = true;
                        cmd = arg;
                    }
                    else {
                        args.push_back(arg);
                    }
                }
                if (cmd == "exit") isStarted = false;
                if (cmd == "history") print_history();
                if (cmd == "cls") cls();
                if (cmd == "exception") throw runtime_error("TEST ERROR");
                if (cmd == "portcheck") portchecker(args);
            }
            catch (const exception& exception) {
                cout << "\033[1;31mУпс.. Произошла непредвиденая ошибка" << endl;
            }
        }
        cout << "\033[1;31mExiting...\033[0m";
    }
};



auto run_and_time(std::function<void()> fn)
{
//    std::chrono::high_resolution_clock timer;
//    auto t0 = timer.now();
//    auto t1 = timer.now();
    auto t0 = std::chrono::system_clock::now();

    fn();

    auto t1 = std::chrono::system_clock::now();
    return t1 - t0;
//    auto diff_ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
}

long int time_ms(auto diff)
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
}

void test1()
{
    int a = 0;
    for (int i = 0; i < 100000; i++)
    {
        a += i * 2;
    }
}

int main() {

//    auto time_test1 = run_and_time(test1);
//    auto time_test1_ms = time_ms(time_test1);

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    AmoreHundEvelis hundEvelis;
    exit(0);
}

