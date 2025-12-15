//
// Created by Amore (ALC) on 13.12.2025.
// Update by Amore (ALC) on 14.12.2025
//

#include <chrono>
#include <functional>
#include <windows.h>
#include <iostream>
#include "hund_evelis_home.h"


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

    auto time_test1 = run_and_time(test1);
    auto time_test1_ms = time_ms(time_test1);

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    AmoreHundEvelis hundEvelis;
    exit(0);
}

