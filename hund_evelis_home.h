//
// Created by Amore (ALC) on 13.12.2025.
// Update by Amore (ALC) on 14.12.2025
//

#pragma once

#include <iostream>
#include <string>
#include <stdlib.h>


using namespace std;



class AmoreHundEvelis {
private:
    string nameUser;
    bool isStarted = true;
public:
    AmoreHundEvelis() {
        char* _nameUser = getenv("USERNAME");
        nameUser =  _nameUser ? _nameUser : "localuser";
        print_welcome_text();
        while_event_handler();
    }

    void print_welcome_text() {
        cout << "\033[32mAmore PiRasNat\033[0m \033[1;31m5\033[0m | \033[34mWELCOME BACK\033[0m\n\n" << endl << endl;
    }

    void clear_screen() {
        system("cls");
    }

    void while_event_handler() {
        while (isStarted) {
            cout << "\033[34m" << nameUser << ":\033[0m \033[32m$\033[0m\033[34m~\033[0m ";
            string input;
            getline(cin, input);
            if (input.empty()) continue;
            if (input == "exit") isStarted = false;
            if (input == "cls") clear_screen();
        }
        cout << "\033[1;31mExiting...";
        exit(0);
    }
};
