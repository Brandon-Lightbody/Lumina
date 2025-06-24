#include "Engine/Core/Log.h"
#include <chrono>
#include <iomanip>
#include <ctime>
#include <filesystem>

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace Engine::Core {
    // Static member initialization
    std::vector<std::unique_ptr<ILogBackend>> Log::s_Backends;
    std::mutex Log::s_Mutex;
    std::atomic<bool> Log::s_Initialized(false);

    void Log::Initialize() {
        if (s_Initialized.exchange(true)) return;

        std::lock_guard lock(s_Mutex);
        try {
            // Create logs directory
            std::filesystem::create_directories("logs");

            // Add default backends
            s_Backends.push_back(std::make_unique<ConsoleBackend>());
            s_Backends.push_back(std::make_unique<FileBackend>("logs/engine.log"));

            Info("Logging system initialized");
        }
        catch (const std::exception& e) {
            // Fallback to console only
            s_Backends.push_back(std::make_unique<ConsoleBackend>());
            Error("Log initialization failed: {}", e.what());
        }
    }

    void Log::Shutdown() {
        if (!s_Initialized) return;

        std::lock_guard lock(s_Mutex);
        Flush();
        s_Backends.clear();
        s_Initialized = false;
        Info("Logging system shutdown complete");
    }

    void Log::AddBackend(std::unique_ptr<ILogBackend> backend) {
        std::lock_guard lock(s_Mutex);
        s_Backends.push_back(std::move(backend));
    }

    void Log::Flush() {
        std::lock_guard lock(s_Mutex);
        for (auto& backend : s_Backends) {
            backend->Flush();
        }
    }

    // Console backend implementation
    void ConsoleBackend::Write(LogLevel level, const std::string& message) {
#ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
        GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
        WORD originalColor = consoleInfo.wAttributes;

        WORD color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; // Default: white
        switch (level) {
        case LogLevel::Trace:    color = FOREGROUND_INTENSITY; break;
        case LogLevel::Debug:    color = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
        case LogLevel::Info:     color = FOREGROUND_GREEN; break;
        case LogLevel::Warn:     color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
        case LogLevel::Error:    color = FOREGROUND_RED | FOREGROUND_INTENSITY; break;
        case LogLevel::Critical: color = BACKGROUND_RED | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY; break;
        }

        SetConsoleTextAttribute(hConsole, color);
        fmt::print("{}\n", message);
        SetConsoleTextAttribute(hConsole, originalColor);
#else
        // Use fmt's built-in colors for other platforms
        fmt::text_style style;
        switch (level) {
        case LogLevel::Trace:    style = fg(fmt::color::gray); break;
        case LogLevel::Debug:    style = fg(fmt::color::cyan); break;
        case LogLevel::Info:     style = fg(fmt::color::green); break;
        case LogLevel::Warn:     style = fg(fmt::color::yellow) | fmt::emphasis::bold; break;
        case LogLevel::Error:    style = fg(fmt::color::red) | fmt::emphasis::bold; break;
        case LogLevel::Critical: style = bg(fmt::color::red) | fg(fmt::color::white) | fmt::emphasis::bold; break;
        }

        fmt::print(style, "{}\n", message);
#endif
    }

    void ConsoleBackend::Flush() {
        fflush(stdout);
    }

    // File backend implementation
    FileBackend::FileBackend(const std::string& filename) {
        // Create log file with timestamp
        auto now = std::chrono::system_clock::now();
        auto time_c = std::chrono::system_clock::to_time_t(now);
        std::tm tm;
#ifdef _WIN32
        localtime_s(&tm, &time_c);
#else
        localtime_r(&time_c, &tm);
#endif

        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
        std::string timestamp = oss.str();

        size_t pos = filename.find_last_of('.');
        std::string name = (pos != std::string::npos) ? filename.substr(0, pos) : filename;
        std::string ext = (pos != std::string::npos) ? filename.substr(pos) : "";

        std::string fullpath = fmt::format("{}-{}{}", name, timestamp, ext);

        file.open(fullpath, std::ios::app);
        if (!file.is_open()) {
            throw std::runtime_error(fmt::format("Failed to open log file: {}", fullpath));
        }

        // Write header
        auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
        auto ms = now_ms.time_since_epoch().count() % 1000;

        file << fmt::format("[{:%Y-%m-%d %H:%M:%S}.{:03d}] [INFO] Log file created\n",
            tm, ms);
    }

    void FileBackend::Write(LogLevel level, const std::string& message) {
        if (!file.is_open()) return;

        auto now = std::chrono::system_clock::now();
        auto time_c = std::chrono::system_clock::to_time_t(now);
        std::tm tm;
#ifdef _WIN32
        localtime_s(&tm, &time_c);
#else
        localtime_r(&time_c, &tm);
#endif

        auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
        auto ms = now_ms.time_since_epoch().count() % 1000;

        static const char* levelStrings[] = {
            "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL"
        };

        file << fmt::format("[{:%Y-%m-%d %H:%M:%S}.{:03d}] [{}] {}\n",
            tm, ms, levelStrings[static_cast<int>(level)], message);
    }

    void FileBackend::Flush() {
        if (file.is_open()) {
            file.flush();
        }
    }
}