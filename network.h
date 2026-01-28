#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <thread>
#include <mutex>
#include <chrono>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <future>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

class NetworkTools {
private:
    mutex cout_mutex;
    mutex queue_mutex;
    condition_variable queue_cv;
    vector<int> open_ports;

    // Очередь для отложенного вывода
    struct OutputItem {
        string message;
        bool is_open;
        int port;
    };
    queue<OutputItem> output_queue;
    thread output_thread;
    atomic<bool> output_running{false};
    atomic<bool> show_close_ports{true};
    atomic<int> pending_outputs{0};

    static bool winsockInitialized;
    static mutex winsockMutex;

    static void initWinsock() {
        lock_guard<mutex> lock(winsockMutex);
        if (!winsockInitialized) {
            WSADATA wsaData;
            int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
            if (result != 0) {
                cerr << "WSAStartup failed with error: " << result << endl;
                throw runtime_error("Winsock initialization failed");
            }
            winsockInitialized = true;
        }
    }

    // Поток для плавного вывода (НЕ static)
    void output_worker() {
        while (output_running || pending_outputs > 0) {
            OutputItem item;
            bool has_item = false;

            {
                unique_lock<mutex> lock(queue_mutex);
                if (queue_cv.wait_for(lock, chrono::milliseconds(100),
                                      [this]() { return !output_queue.empty() || !output_running; })) {

                    if (!output_queue.empty()) {
                        item = output_queue.front();
                        output_queue.pop();
                        has_item = true;
                    }
                }
            }

            if (has_item) {
                lock_guard<mutex> lock(cout_mutex);
                if (item.is_open) {
                    cout << "\033[1;32mПорт " << item.port << " открыт\033[0m" << endl;
                } else {
                    cout << "\033[31mПорт " << item.port << " закрыт\033[0m" << endl;
                }
                pending_outputs--;

                // Задержка только для закрытых портов при выводе
                if (!item.is_open && show_close_ports) {
                    this_thread::sleep_for(chrono::microseconds(3000)); // 3ms
                }
            }
        }
    }

    // Добавление в очередь вывода
    void queue_output(int port, bool is_open) {
        {
            lock_guard<mutex> lock(queue_mutex);
            output_queue.push({is_open ? "open" : "closed", is_open, port});
            pending_outputs++;
        }
        queue_cv.notify_one();
    }

    // Проверка подключения к порту (не static)
    bool connect_to_port(const string& ip, int port, int timeout_ms = 1000) {
        initWinsock();

        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) {
            cerr << "\033[1;31mОшибка создания сокета: " << WSAGetLastError() << "\033[0m" << endl;
            return false;
        }

        // Устанавливаем неблокирующий режим для таймаута
        u_long mode = 1;
        ioctlsocket(sock, FIONBIO, &mode);

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);

        // Преобразуем IP-адрес
        if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) != 1) {
            cerr << "\033[1;31mОшибка: неверный IP-адрес\033[0m" << endl;
            closesocket(sock);
            return false;
        }

        connect(sock, (sockaddr*)&addr, sizeof(addr));

        fd_set fdset;
        FD_ZERO(&fdset);
        FD_SET(sock, &fdset);

        timeval tv{};
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;

        bool connected = false;
        if (select(0, nullptr, &fdset, nullptr, &tv) > 0) {
            int so_error;
            socklen_t len = sizeof(so_error);
            getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&so_error, &len);
            connected = (so_error == 0);
        }

        closesocket(sock);
        return connected;
    }

public:
    // Конструктор без аргументов для тестирования
    NetworkTools() {
        initWinsock();
        // Запускаем поток вывода
        output_running = true;
        output_thread = thread(&NetworkTools::output_worker, this);
    }

    // Деструктор
    ~NetworkTools() {
        // Останавливаем поток вывода
        output_running = false;
        queue_cv.notify_all();
        if (output_thread.joinable()) {
            output_thread.join();
        }
    }

    // Конструктор с аргументами (старый)
    explicit NetworkTools(vector<string> args) : NetworkTools() {
        if (args.empty()) {
            cout << "\x1b[1;31mОшибка: нет аргументов. \x1b[0mИспользуйте:" << endl;
            cout << "  gip <hostname>" << endl;
            cout << "  poc <ip> <start> <end> [-rf]" << endl;
            return;
        }

        if (args[0] == "PortOpenCheck" || args[0] == "poc") {
            if (args.size() < 4) {
                cout << "\x1b[1;31mОшибка: для poc нужно указать IP, начальный и конечный порт" << endl;
                return;
            }
            portcheck(args);
        }
        else if (args[0] == "GetIPInURL" || args[0] == "gip") {
            if (args.size() < 2) {
                cout << "\x1b[1;31mОшибка: для gip нужно указать хостнейм" << endl;
                return;
            }
            gip(args);
        }
        else {
            cout << "\x1b[1;31mНеизвестная команда: " << args[0] << endl;
        }
    }

    // Метод для выполнения команд из интерактивного режима
    void executeCommand(const string& command) {
        vector<string> args;
        string token;
        size_t start = 0, end = 0;

        // Разбиваем строку на аргументы
        while ((end = command.find(' ', start)) != string::npos) {
            token = command.substr(start, end - start);
            if (!token.empty()) args.push_back(token);
            start = end + 1;
        }
        token = command.substr(start);
        if (!token.empty()) args.push_back(token);

        if (args.empty()) {
            cout << "Введите команду (gip или poc)" << endl;
            return;
        }

        if (args[0] == "poc" || args[0] == "PortOpenCheck") {
            if (args.size() < 4) {
                cout << "Использование: poc <IP> <начальный_порт> <конечный_порт> [-rf]" << endl;
                cout << "Пример: poc 127.0.0.1 1 100" << endl;
                return;
            }
            portcheck(args);
        }
        else if (args[0] == "gip" || args[0] == "GetIPInURL") {
            if (args.size() < 2) {
                cout << "Использование: gip <хостнейм>" << endl;
                cout << "Пример: gip google.com" << endl;
                return;
            }
            gip(args);
        }
        else if (args[0] == "help") {
            showHelp();
        }
        else {
            cout << "Неизвестная команда: " << args[0] << endl;
            showHelp();
        }
    }

    static void showHelp() {
        cout << "Доступные команды:" << endl;
        cout << "  gip <hostname>              - получить IP-адреса хоста" << endl;
        cout << "  poc <ip> <start> <end> [-rf] - сканировать порты" << endl;
        cout << "    -rf - показывать только открытые порты" << endl;
        cout << "  help                        - показать эту справку" << endl;
        cout << "  exit                        - выйти" << endl;
    }

    void executeCommandFromArgs(const vector<string>& args) {
        if (args.empty()) {
            showHelp();
            return;
        }

        if (args[0] == "gip" || args[0] == "GetIPInURL") {
            if (args.size() < 2) {
                cout << "Использование: gip <хостнейм>" << endl;
                return;
            }
            gip(args);
        }
        else if (args[0] == "poc" || args[0] == "PortOpenCheck") {
            if (args.size() < 4) {
                cout << "Использование: poc <IP> <начальный_порт> <конечный_порт> [-rf]" << endl;
                return;
            }
            portcheck(args);
        }
        else {
            cout << "Неизвестная команда: " << args[0] << endl;
            showHelp();
        }
    }

    static void gip(vector<string> args) {
        initWinsock();

        if (args.size() < 2) {
            cerr << "Usage: gip <hostname>" << endl;
            return;
        }

        const std::string& hostnameStr = args[1];
        const char* hostname = hostnameStr.c_str();

        std::cout << "Смотрим IP-адреса: " << hostnameStr << std::endl;

        struct addrinfo hints{}, *result = nullptr, *rp;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_CANONNAME;

        int status = getaddrinfo(hostname, nullptr, &hints, &result);
        if (status != 0) {
            std::cerr << "Ошибка: " << gai_strerror(status) << std::endl;
            return;
        }

        std::vector<std::string> ipv4Addresses;
        std::vector<std::string> ipv6Addresses;

        for (rp = result; rp != nullptr; rp = rp->ai_next) {
            char ip_str[INET6_ADDRSTRLEN];
            void* addr;

            if (rp->ai_family == AF_INET) {
                auto* ipv4 = (struct sockaddr_in*)rp->ai_addr;
                addr = &(ipv4->sin_addr);
                inet_ntop(AF_INET, addr, ip_str, sizeof(ip_str));
                ipv4Addresses.emplace_back(ip_str);
            } else if (rp->ai_family == AF_INET6) {
                auto* ipv6 = (struct sockaddr_in6*)rp->ai_addr;
                addr = &(ipv6->sin6_addr);
                inet_ntop(AF_INET6, addr, ip_str, sizeof(ip_str));
                ipv6Addresses.emplace_back(ip_str);
            }
        }

        if (!ipv4Addresses.empty()) {
            std::cout << "\nIPv4 адреса:" << std::endl;
            for (const auto& ip : ipv4Addresses) {
                std::cout << "  " << ip << std::endl;
            }
        }

        if (!ipv6Addresses.empty()) {
            std::cout << "\nIPv6 адреса:" << std::endl;
            for (const auto& ip : ipv6Addresses) {
                std::cout << "  " << ip << std::endl;
            }
        }

        if (ipv4Addresses.empty() && ipv6Addresses.empty()) {
            std::cout << "IP-адреса не найдены" << std::endl;
        } else {
            std::cout << "\nНайдено адресов: "
                      << (ipv4Addresses.size() + ipv6Addresses.size())
                      << std::endl;
        }

        freeaddrinfo(result);
    }

    void portcheck(vector<string> args) {
        if (args.size() != 4 && args.size() != 5) {
            cout << "Использование: poc <IP> <начальный_порт> <конечный_порт> [-rf]" << endl;
            cout << "  -rf - не показывать закрытые порты (опционально)" << endl;
            return;
        }

        if (args.size() == 4) {
            check_last(args[1], args[2], args[3]);
        } else if (args.size() == 5) {
            check_last(args[1], args[2], args[3], args[4]);
        }
    }

    void check_last(const string& ip, const string& port_start, const string& port_end, const string& arg = "-rf") {
        int really_port_start;
        int really_port_end;

        try {
            if (port_start == "*") {
                really_port_start = 1;
            } else {
                really_port_start = stoi(port_start);
                if (really_port_start < 1 || really_port_start > 65535) {
                    throw out_of_range("Порт должен быть от 1 до 65535");
                }
            }

            if (port_end == "*") {
                really_port_end = 65535;
            } else {
                really_port_end = stoi(port_end);
                if (really_port_end < 1 || really_port_end > 65535) {
                    throw out_of_range("Порт должен быть от 1 до 65535");
                }
            }

            if (really_port_start > really_port_end) {
                swap(really_port_start, really_port_end);
            }

            if (arg == "-rf") {
                show_close_ports = false;
            } else {
                show_close_ports = true;
            }

            scan_range(ip, really_port_start, really_port_end);

        } catch (const invalid_argument& e) {
            cerr << "Ошибка: неверный формат порта" << endl;
        } catch (const out_of_range& e) {
            cerr << "\033[1;31mОшибка\033[0m" << endl;
        }
    }

    void scan_port(const string& ip, int port) {
        try {
            bool is_open = connect_to_port(ip, port);

            if (is_open) {
                open_ports.push_back(port);
                queue_output(port, true);
            } else if (show_close_ports) {
                queue_output(port, false);
            }
        } catch (const exception& e) {
            cerr << "Ошибка при сканировании порта " << port << ": " << e.what() << endl;
        }
    }

    void scan_range(const std::string& ip, int start_port, int end_port, int max_threads = 100) {
        cout << "Сканирование " << ip << " с порта " << start_port << " по " << end_port << "..." << endl;

        open_ports.clear();

        vector<thread> threads;
        atomic<int> current_port{start_port};
        int total_ports = end_port - start_port + 1;

        // Ограничиваем количество одновременно работающих потоков
        int active_threads = min(max_threads, total_ports);

        for (int i = 0; i < active_threads; i++) {
            threads.emplace_back([this, ip, &current_port, end_port]() {
                while (true) {
                    int port = current_port++;
                    if (port > end_port) break;

                    this->scan_port(ip, port);

                    // Небольшая пауза между портами в одном потоке
                    this_thread::sleep_for(chrono::microseconds(500));
                }
            });
        }

        // Ожидаем завершения всех потоков сканирования
        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }

        // Ждем, пока все выводы завершатся
        while (pending_outputs > 0) {
            this_thread::sleep_for(chrono::milliseconds(100));
        }

        cout << "\nСканирование завершено." << endl;

        if (!open_ports.empty()) {
            cout << "\nОткрытые порты (" << open_ports.size() << "):" << endl;
            sort(open_ports.begin(), open_ports.end());
            for (int port : open_ports) {
                cout << "  \033[32mПорт: " << port << "\033[0m" << endl;
            }
        } else {
            cout << "\033[1;33mОткрытые порты не обнаружены!\033[0m" << endl;
        }
    }
};

// Инициализация статических переменных
inline bool NetworkTools::winsockInitialized = false;
inline mutex NetworkTools::winsockMutex;