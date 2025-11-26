#pragma once

#include <memory>
#include <string>
#include <mutex>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

class Log
{
public:
    // Initialize the logger
    static void init(const std::string& log_dir = "src/log/",
                     spdlog::level::level_enum log_level = spdlog::level::info,
                     const std::string& project_root = "");

    // Log a message (automatically adds file, line, and module tag)
    static void log(spdlog::level::level_enum level,
                    const std::string& message,
                    const char* file,
                    int line,
                    const std::string& module = "",
                    bool to_console = true);

private:
    // Logger components
    static std::shared_ptr<spdlog::logger> s_logger;
    static std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> s_console_sink;
    static std::shared_ptr<spdlog::sinks::basic_file_sink_mt> s_file_sink;
    static std::mutex s_mutex;

    // Paths and filenames
    static std::string s_project_root;
    static std::string s_log_dir;
    static std::string s_base_filename;
    static std::string s_base_filename_root;   // <- Root filename (no _1, _2)
    static size_t s_file_index;

    // Rotation settings
    static constexpr size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB

    // Helpers
    static std::string generate_log_filename(const std::string& log_dir);
    static std::string trim_project_path(const std::string& full_path);
    static void rotate_if_needed();
};

#define LOG_INFO(msg, module)  Log::log(spdlog::level::info, msg, __FILE__, __LINE__, module)
#define LOG_DEBUG(msg, module) Log::log(spdlog::level::debug, msg, __FILE__, __LINE__, module)
#define LOG_WARN(msg, module)  Log::log(spdlog::level::warn, msg, __FILE__, __LINE__, module)
#define LOG_ERROR(msg, module) Log::log(spdlog::level::err, msg, __FILE__, __LINE__, module)
#define LOG_FILE_ONLY(msg, module) Log::log(spdlog::level::info, msg, __FILE__, __LINE__, module, false)
