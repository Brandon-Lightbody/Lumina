#pragma once
#include <string>
#include <memory>
#include <mutex>
#include <atomic>
#include <vector>
#include <fstream>
#include <fmt/format.h>
#include <fmt/chrono.h>
#include <fmt/core.h>
#include <fmt/color.h>

// Workaround for MSVC constexpr issues
#if defined(_MSC_VER)
#define FMT_CONSTEXPR
#else
#define FMT_CONSTEXPR constexpr
#endif

namespace Engine::Core {
    // Log level enumeration
    enum class LogLevel {
        Trace,
        Debug,
        Info,
        Warn,
        Error,
        Critical
    };

    // Abstract logger backend interface
    class ILogBackend {
    public:
        virtual ~ILogBackend() = default;
        virtual void Write(LogLevel level, const std::string& message) = 0;
        virtual void Flush() = 0;
    };

    // Main logger class
    class Log {
    public:
        static void Initialize();
        static void Shutdown();

        // Logging functions
        template<typename... Args>
        static void Trace(FMT_CONSTEXPR fmt::format_string<Args...> fmt, Args&&... args) {
            Write(LogLevel::Trace, fmt, std::forward<Args>(args)...);
        }

        template<typename... Args>
        static void Debug(FMT_CONSTEXPR fmt::format_string<Args...> fmt, Args&&... args) {
            Write(LogLevel::Debug, fmt, std::forward<Args>(args)...);
        }

        template<typename... Args>
        static void Info(FMT_CONSTEXPR fmt::format_string<Args...> fmt, Args&&... args) {
            Write(LogLevel::Info, fmt, std::forward<Args>(args)...);
        }

        template<typename... Args>
        static void Warn(FMT_CONSTEXPR fmt::format_string<Args...> fmt, Args&&... args) {
            Write(LogLevel::Warn, fmt, std::forward<Args>(args)...);
        }

        template<typename... Args>
        static void Error(FMT_CONSTEXPR fmt::format_string<Args...> fmt, Args&&... args) {
            Write(LogLevel::Error, fmt, std::forward<Args>(args)...);
        }

        template<typename... Args>
        static void Critical(FMT_CONSTEXPR fmt::format_string<Args...> fmt, Args&&... args) {
            Write(LogLevel::Critical, fmt, std::forward<Args>(args)...);
        }

        static void AddBackend(std::unique_ptr<ILogBackend> backend);
        static void Flush();

    private:
        template<typename... Args>
        static void Write(LogLevel level, FMT_CONSTEXPR fmt::format_string<Args...> fmt, Args&&... args) {
            if (!s_Initialized) return;

            try {
                std::string formatted = fmt::vformat(fmt, fmt::make_format_args(args...));
                std::lock_guard lock(s_Mutex);
                for (auto& backend : s_Backends) {
                    backend->Write(level, formatted);
                }
            }
            catch (const fmt::format_error& e) {
                // Fallback to simple output
                std::lock_guard lock(s_Mutex);
                for (auto& backend : s_Backends) {
                    backend->Write(LogLevel::Error,
                        fmt::format("Log formatting error: {}", e.what()));
                }
            }
        }

        static std::vector<std::unique_ptr<ILogBackend>> s_Backends;
        static std::mutex s_Mutex;
        static std::atomic<bool> s_Initialized;
    };

    // Concrete backends
    class ConsoleBackend : public ILogBackend {
    public:
        void Write(LogLevel level, const std::string& message) override;
        void Flush() override;
    };

    class FileBackend : public ILogBackend {
    public:
        FileBackend(const std::string& filename);
        void Write(LogLevel level, const std::string& message) override;
        void Flush() override;

    private:
        std::ofstream file;
    };
}

// Logging macros with source location
#define LOG_INTERNAL(level, ...) \
    do { \
        std::string formatted = fmt::format(__VA_ARGS__); \
        formatted += fmt::format(" [{}:{}]", __FILE__, __LINE__); \
        Engine::Core::Log::level("{}", formatted); \
    } while(0)

#define LOG_TRACE(...)    LOG_INTERNAL(Trace, __VA_ARGS__)
#define LOG_DEBUG(...)    LOG_INTERNAL(Debug, __VA_ARGS__)
#define LOG_INFO(...)     Engine::Core::Log::Info(__VA_ARGS__)
#define LOG_WARN(...)     Engine::Core::Log::Warn(__VA_ARGS__)
#define LOG_ERROR(...)    LOG_INTERNAL(Error, __VA_ARGS__)
#define LOG_CRITICAL(...) LOG_INTERNAL(Critical, __VA_ARGS__)