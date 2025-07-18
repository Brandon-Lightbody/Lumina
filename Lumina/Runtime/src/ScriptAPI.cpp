#include "Runtime/ScriptAPI.h"
#include "Runtime/RuntimeExports.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>

void LogToNative(const char* message) {
    try {
        if (message) {
            std::cout << "[Managed] " << message << std::endl;

            std::ofstream logfile("native_log.txt", std::ios::app);
            if (logfile) {
                auto now = std::chrono::system_clock::now();
                auto now_c = std::chrono::system_clock::to_time_t(now);
                logfile << "[" << std::put_time(std::localtime(&now_c), "%F %T")
                    << "] " << message << "\n";
            }
        }
    }
    catch (...) {
        std::cerr << "Exception in LogToNative handler\n";
    }
}