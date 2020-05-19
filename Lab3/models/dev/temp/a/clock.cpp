#include <stdio.h>
#include <time.h>

#include <Windows.h>

#include <chrono>
#include <thread>

double tickrate = 60.0;
double tick_time = 1000.0/tickrate;

std::chrono::system_clock::time_point a = std::chrono::system_clock::now();
std::chrono::system_clock::time_point b = std::chrono::system_clock::now();

int main()
{
    while (true)
    {
        // Maintain designated frequency of 5 Hz (200 ms per frame)
        a = std::chrono::system_clock::now();
        std::chrono::duration<double, std::milli> work_time = a - b;

        if (work_time.count() < tick_time)
        {
            std::chrono::duration<double, std::milli> delta_ms(tick_time - work_time.count());
            auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
            Sleep(delta_ms_duration.count());
        }

        b = std::chrono::system_clock::now();
        std::chrono::duration<double, std::milli> sleep_time = b - a;

        // Your code here

        printf("Time: %f \n", (work_time + sleep_time).count());
    }
}