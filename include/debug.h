#pragma once

#include <iostream>
#include <mutex>
#include <thread>

#define YELLOW      "\u001b[33;1m"
#define RED         "\u001b[31;1m"
#define PURPLE      "\u001b[35;1m"
#define COLOR_END   "\u001b[0m"

extern std::mutex io_mutex;

#define DW(x) do { io_mutex.lock(); x; io_mutex.unlock(); } while (0)
#define COLOR(color, x) color << x << COLOR_END

#define DEBUG(x) DW(std::cout << COLOR(YELLOW, "[DEBUG] ") << x << '\n')
#define WARN(x) DW(std::cout << COLOR(PURPLE, "[WARNING] ") << x << '\n')
#define ERROR(x) DW(std::cout << COLOR(RED, "[ERROR] ") << x << '\n')