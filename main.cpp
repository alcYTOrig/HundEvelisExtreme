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
#include <utility>
#include <vector>
#include <functional>
#include <windows.h>
#include <chrono>
#include <future>

#include "network.h"

using namespace std;

class AmoreHundEvelis {
private:
    string nameUser;
    bool isStarted = true;
    NetworkTools* networkTools = nullptr;  // Указатель на NetworkTools
    map<int, string> historyCommands_and_id;
    int history_last_id = 0;
    map<int, long long> commandTime;
public:
    AmoreHundEvelis() {
        char* _nameUser = getenv("USERNAME");
        nameUser =  _nameUser ? _nameUser : "localuser";

        int code_checker = process_checker();
        if (code_checker == 0) {
            cls();
            print_welcome_text();
            while_event_handler();
        }
        else {
            cout << "\033[1;31mExiting...\033[0m";
            exit(0);
        }
    }

    ~AmoreHundEvelis() {
        if (networkTools != nullptr) {
            delete networkTools;
        }
    }

    static void print_of_console(char access, const string& text) {
        cout << "[\033[1;31m" << access << "\033[0m]" << " \033[4;35m" << text << "\033[0m";
    }

    int process_checker() {
        bool error = false;
        try {
            networkTools = new NetworkTools();
        } catch (const exception& e) {
            error = true;
            print_of_console('-', "Ошибка инициализации сетевых инструментов");
            networkTools = nullptr;
        }
        if (error) {
            cout << endl << endl << "\033[1;31mПри запуске возникла ошибка при инициализации!" << endl << "Введите '\033[1;32mStartOutService'\033[1;31m для пропуска ошибки!: ";
            string input_text;
            cin >> input_text;
            if (input_text == "StartOutService") return 0;
            else {
                return 1;
            }
        }
        else return 0;
    }

    static vector<string> split(const string& str, char delimiter) {
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

    static void print_welcome_text() {
        cout << "\x1b[1;32mAmore \033[1;34mHundEvelis\033[0m\033[1;31mExtreme\033[0m | \033[1;34mWELCOME BACK\033[0m\n";
        cout << "\x1b[1;34mAlpha \x1b[0m\x1b[1mV2.2\x1b[0m" << endl << endl << endl;
    }

    static void cls() {
        system("cls");
    }

    void print_history() {
        for (const auto& cmd_local : historyCommands_and_id) {
            cout << "ID: " << cmd_local.first << " - " << "'" << cmd_local.second << "'" << endl;
        }
    }

    void show_more_info_command(vector<string> args) {
        if (args.empty()) {
            cout << "Ошибка: не указан ID команды" << endl;
            cout << "Использование: gmifc <ID> [формат]" << endl;
            cout << "Форматы: -ns (наносекунды), -us (микросекунды), -ms (миллисекунды), -s (секунды)" << endl;
            return;
        }

        int id;
        try {
            id = stoi(args[0]);
        } catch (...) {
            cout << "Ошибка: некорректный ID" << endl;
            return;
        }

        // Проверяем существование команды
        if (historyCommands_and_id.find(id) == historyCommands_and_id.end()) {
            cout << "Ошибка: команда с ID " << id << " не найдена" << endl;
            return;
        }

        string command_name = historyCommands_and_id[id];

        // Проверяем наличие времени для команды
        if (commandTime.find(id) == commandTime.end()) {
            cout << "Информация:" << endl;
            cout << "Name: " << command_name << endl;
            cout << "ID: " << id << endl;
            cout << "Time complete: не измерялось" << endl;
            return;
        }

        long long time_ns = commandTime[id];

        cout << "Input: " << command_name << endl;
        cout << "ID: " << id << endl;

        // Определяем формат вывода
        string time_format = "auto";
        if (args.size() > 1) {
            time_format = args[1];
        }

        // Форматируем время
        if (time_format == "-ns") {
            cout << "Time complete: " << time_ns << "ns" << endl;
        }
        else if (time_format == "-ms") {
            cout << "Time complete: " << time_ns / 1000000.0 << "ms" << endl;
        }
        else if (time_format == "-s") {
            cout << "Time complete: " << time_ns / 1000000000.0 << "s" << endl;
        }
        else if (time_format == "-h") {
            cout << "Time complete: " << time_ns / 60000000000.0 << "h" << endl;
        }
        else {
            // Автоформатирование
            if (time_ns < 1000) {
                cout << "Time complete: " << time_ns << "ns" << endl;
            }
            else if (time_ns < 1000000000) {
                cout << "Time complete: " << time_ns / 1000000.0 << "ms" << endl;
            }
            else if (time_ns < 60000000000){
                cout << "Time complete: " << time_ns / 1000000000.0 << "s" << endl;
            }
            else {
                cout << "Time complete: " << time_ns / 60000000000.0 << "h" << endl;
            }
        }
    }

    void portchecker(const vector<string>& args_local) {
        if (networkTools == nullptr) {
            cout << "\033[1;31mСетевые инструменты не инициализированы!\033[0m" << endl;
            return;
        }

        if (args_local.empty()) {
            NetworkTools::showHelp();
            return;
        }

        vector<string> fullArgs = args_local;
        if (!args_local.empty() && (args_local[0] != "gip" && args_local[0] != "poc")) {
            fullArgs.insert(fullArgs.begin(), "gip");
        }

        networkTools->executeCommandFromArgs(fullArgs);
    }

    static void help() {
        cout << "\x1b[35mДокументация по HundEvelis доступна на сайте \x1b[32mhttps://alc1.ru/amore/hundevelis/document.html\x1b[0m" << endl;
    }

    void run_and_time(const std::function<void()>& fn) {
        auto t0 = std::chrono::steady_clock::now();
        fn();
        auto t1 = std::chrono::steady_clock::now();

        if (t1 < t0) {
            std::swap(t0, t1);
        }

        auto duration = t1 - t0;
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);

        long long ns_count = std::llabs(ns.count());

        commandTime[history_last_id] = ns_count;
        history_last_id++;  // ДОБАВЬТЕ ЭТУ СТРОЧКУ
    }

    void run_and_time_with_arguments(const std::function<void(std::vector<std::string>)>& fn,
                                     std::vector<std::string> args) {
        auto t0 = std::chrono::steady_clock::now();
        fn(std::move(args));
        auto t1 = std::chrono::steady_clock::now();

        if (t1 < t0) {
            std::swap(t0, t1);
        }

        auto duration = t1 - t0;
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
        long long ns_count = std::llabs(ns.count());

        commandTime[history_last_id] = ns_count;
        history_last_id++;  // ДОБАВЬТЕ ЭТУ СТРОЧКУ
    }

    void while_event_handler() {
        while (isStarted) {
            try {
                cout << "\033[35m" << nameUser << "\033[34m:\033[0m \033[32m$\033[0m\033[34m~\033[0m ";
                string input;
                getline(cin, input);
                if (input.empty()) continue;

                vector<string> command;
                vector<string> args;
                string cmd;

                command = split(input, ' ');
                args.clear();

                if (command.empty()) continue;

                cmd = command[0];

                for (size_t i = 1; i < command.size(); i++) {
                    args.push_back(command[i]);
                }

                // Флаг для отслеживания, была ли команда обработана
                bool command_handled = false;

                if (cmd == "exit") {
                    isStarted = false;
                    command_handled = true;
                }
                else if (cmd == "history") {
                    print_history();
                    command_handled = true;
                }
                else if (cmd == "cls") {
                    run_and_time([](){cls();});
                    command_handled = true;
                }
                else if (cmd == "exception") {
                    throw runtime_error("TEST ERROR");
                }
                else if (cmd == "net") {
                    run_and_time_with_arguments([this](const vector<string>& args_param) {
                        portchecker(args_param);
                    }, args);
                    command_handled = true;
                }
                else if (cmd == "help") {
                    run_and_time([](){help();});
                    command_handled = true;
                }
                else if (cmd == "gmifc") {
                    show_more_info_command(args);
                    command_handled = true;
                }
                else if (cmd == "ssm") {
                    if (args.empty()) {
                        run_and_time([](){print_welcome_text();});
                        command_handled = true;
                    }
                    else if (args[0] == ">>cls") {
                        run_and_time([](){cls();print_welcome_text();});
                        command_handled = true;
                    }
                    else {
                        cout << "\033[1;33mНеизвестный аргумент для ssm: " << args[0] << "\033[0m" << endl;
                    }
                }
                else if (cmd == "shut") {
                    if (args.empty()) {
                        system("shutdown /s /t 5");
                        cout << "Shutdown!" << endl;
                    }
                    else if (args[0] == "-r") {
                        system("shutdown /r /t 5");
                        cout << "Restart!" << endl;
                    }
                }
                else if (cmd == "c") {
                    system("shutdown /a");
                    cout << "Shutdown off!" << endl;
                }
                else if (cmd == "DeepSeek_TOPSECRET") {
                    cout << "Вот что сказал DeepSeek когда я ему сказал сделать ASCII арт: \n";
                    cout << "Ты абсолютно прав! Давай по-честному - мне лень каждый раз делать новый ASCII арт!" << endl;
                }
                else {
                    cout << "\033[1;33mНеизвестная команда: " << cmd << "\033[0m" << endl;
                }

                if (command_handled && cmd != "gmifc" && cmd != "history" &&
                    cmd != "exception" && cmd != "exit") {
                    historyCommands_and_id[history_last_id - 1] = input;
                }
            }
            catch (const exception& e) {
                cout << "\033[1;31mОшибка: " << e.what() << "\033[0m" << endl;
            }
        }
        cout << "\033[1;31mExiting...\033[0m" << endl;
    }
};

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    SetConsoleTitle("HundEvelisExtreme");

    try {
        AmoreHundEvelis hundEvelis;
    } catch (const exception& e) {
        cerr << "\033[1;31mКритическая ошибка: " << e.what() << "\033[0m" << endl;
        return 1;
    }

    return 0;
}