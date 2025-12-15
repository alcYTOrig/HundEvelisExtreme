//
// Created by Amore (ALC) on 13.12.2025.
// Update by Amore (ALC) on 14.12.2025
//

#pragma once

#include <iostream>
#include <string>
#include <map>
#include <algorithm>

using namespace std;


class AmoreHundEvelis {
private:
    string nameUser;
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

    void while_event_handler() {
        while (isStarted) {
            try {
                cout << "\033[34m" << nameUser << ":\033[0m \033[32m$\033[0m\033[34m~\033[0m ";
                string input;
                getline(cin, input);
                if (input.empty()) continue;
                historyCommands_and_id[history_last_id] = input;
                history_last_id++;
                if (input == "exit") isStarted = false;
                if (input == "history") print_history();
                if (input == "cls") cls();
                if (input == "exception") throw runtime_error("TEST ERROR");
            }
            catch (const exception& exception) {
                cout << "\033[1;31mУпс.. Произошла непредвиденая ошибка" << endl;
            }
        }
        cout << "\033[1;31mExiting...\033[0m";
    }
};
