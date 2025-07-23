#include "Runtime/ScriptAPI.h"
#include "Runtime/RuntimeExports.h"

#include <iostream>
#include <fstream>
#include <mutex>

// Single file stream instance with thread safety
static std::ofstream& GetLogFile() {
    static std::ofstream logfile("native_log.txt", std::ios::app);
    return logfile;
}

static std::mutex logMutex;

void LogToNative(const char* message) {
    if (!message) return;

    // Use lock guard for thread safety
    std::lock_guard<std::mutex> lock(logMutex);

    // Faster output without flushing
    std::cout << "[Managed] " << message << '\n';

    // Reuse file stream
    auto& logfile = GetLogFile();
    if (logfile) {
        logfile << message << '\n';
    }
}