#pragma once

#include <iostream>
#include <mutex>
#include <thread>
#include <core.h>

extern std::mutex io_mutex;

#define DEBUG(x) __DW(std::cerr << COLOR(__TERM_YELLOW, "[DEBUG] ") << x << '\n')