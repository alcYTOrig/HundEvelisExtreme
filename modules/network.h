//
// Created by ALC on 20.12.2025.
//

#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <thread>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

class NetworkTools {
private:
    mutex cout_mutex;
    vector<int> open_ports;
public:
    NetworkTools(vector<string> args) {
        int vector_sum = 0;
        for (const auto& el : args) {
            vector_sum++;
        }
        if (vector_sum != 4 && vector_sum != 3) {
            cout << "\033[1;31mНе указаны нужные аргументы!\033[0m" << endl;
        }
        else {
            if (vector_sum == 3) {
                check_last(args[0], args[1], args[2]);
            }
            if (vector_sum == 4) {
                check_last(args[0], args[1], args[2], args[3]);
            }
        }
    }

    void check_last(const string& ip, const string& port_start, const string& port_end, const string& arg = "-rf") {
        int really_port_start;
        int really_port_end;
        bool close_port_cout;
        if (port_start == "*") {
            really_port_start = 1;
        }
        else {
            really_port_start = stoi(port_start);
        }
        if (port_end == "*") {
            really_port_end = 65535;
        }
        else {
            really_port_end = stoi(port_end);
        }
        if (arg == "-rf") {
            close_port_cout = false;
        }
        else {
            close_port_cout = true;
        }
        scan_range(ip, really_port_start, really_port_end, close_port_cout);
    }

    bool connect_to_port(const string& ip, int port, int timeout_ms = 1000) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            return false;
        }

        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) {
            WSACleanup();
            return false;
        }

        // Устанавливаем неблокирующий режим для таймаута
        u_long mode = 1;
        ioctlsocket(sock, FIONBIO, &mode);

        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

        connect(sock, (sockaddr*)&addr, sizeof(addr));

        fd_set fdset;
        FD_ZERO(&fdset);
        FD_SET(sock, &fdset);

        timeval tv;
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;

        bool connected = false;
        if (select(0, NULL, &fdset, NULL, &tv) > 0) {
            int so_error;
            socklen_t len = sizeof(so_error);
            getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&so_error, &len);
            connected = (so_error == 0);
        }

        closesocket(sock);
        WSACleanup();
        return connected;
    }

    void scan_port(const string& ip, int port, bool scp) {
        bool is_open = connect_to_port(ip, port);

        std::lock_guard<std::mutex> lock(cout_mutex); // Всегда блокируем мьютекс

        if (is_open) {
            std::cout << "\033[1;32mПорт " << port << " открыт\033[0m" << std::endl;
            open_ports.push_back(port);
        } else {
            if (scp)
                std::cout << "\033[31mПорт " << port << " закрыт\033[0m" << std::endl;
        }
    }

    void scan_range(const std::string& ip, int start_port, int end_port, bool show_close_ports, int max_threads = 100) {
        std::cout << "Сканирование " << ip << " с порта " << start_port << " по " << end_port << "...\n";

        std::vector<std::thread> threads;
        int current_port = start_port;

        while (current_port <= end_port) {
            while (threads.size() < max_threads && current_port <= end_port) {
                threads.emplace_back([this, ip, current_port, show_close_ports]() {
                    this->scan_port(ip, current_port, show_close_ports);
                });
                current_port++;
            }

            for (auto& t : threads) {
                t.join();
            }
            threads.clear();
        }

        std::cout << "Сканирование завершено.\n" << endl;
        bool open_port = false;
        for (const int& port : open_ports) {
            open_port = true;
            break;
        }
        if (open_port) {
            std::cout << "Открытые порты: " << endl;
            for (const int& port : open_ports) {
                cout << "\033[32mПорт: " << port << endl;
            }
            cout << "\033[0m";
        }
        else {
            cout << "\033[1;32mОткрытые порты не обнаружены!\033[0m";
        }
    }
};
