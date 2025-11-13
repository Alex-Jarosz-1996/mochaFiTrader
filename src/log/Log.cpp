#include "Log.h"
#include <filesystem>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <iostream>

std::shared_ptr<spdlog::logger> Log::s_logger = nullptr;
std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> Log::s_console_sink = nullptr;
std::shared_ptr<spdlog::sinks::basic_file_sink_mt> Log::s_file_sink = nullptr;
std::mutex Log::s_mutex;
std::string Log::s_project_root;
std::string Log::s_log_dir;
std::string Log::s_base_filename;
std::string Log::s_base_filename_root;
size_t Log::s_file_index = 0;
constexpr size_t Log::MAX_FILE_SIZE;

void Log::init(const std::string& log_dir,
               spdlog::level::level_enum log_level,
               const std::string& project_root)
{
    namespace fs = std::filesystem;
    fs::create_directories(fs::path(log_dir));

    s_log_dir = log_dir;
    s_base_filename_root = generate_log_filename(log_dir);
    s_base_filename = s_base_filename_root;
    s_file_index = 0;

    // project root trimming
    s_project_root = !project_root.empty() ? project_root : fs::current_path().string();
    if (s_project_root.back() != '/')
        s_project_root += '/';

    // file sink
    s_file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(s_base_filename, true);
    s_file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] %v");

    // console sink
    s_console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    s_console_sink->set_pattern("[%H:%M:%S] [%^%l%$] [%s:%#] %v");

    std::vector<spdlog::sink_ptr> sinks = { s_file_sink, s_console_sink };
    s_logger = std::make_shared<spdlog::logger>("multi_sink_logger", sinks.begin(), sinks.end());
    s_logger->set_level(log_level);
    s_logger->flush_on(spdlog::level::info);

    spdlog::set_default_logger(s_logger);
    s_logger->info("Logger initialized (file: {})", s_base_filename);
}

std::string Log::generate_log_filename(const std::string& log_dir)
{
    namespace fs = std::filesystem;
    fs::create_directories(log_dir);

    // current time
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);

    std::tm local_tm{};
#ifdef _WIN32
    localtime_s(&local_tm, &t);
#else
    localtime_r(&t, &local_tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%Y-%m-%d-%H-%M-%S");
    std::string timestamp = oss.str();

    // e.g. logs/2025-11-13-17-02-32.txt
    return (fs::path(log_dir) / (timestamp + ".txt")).string();
}

void Log::rotate_if_needed()
{
    namespace fs = std::filesystem;

    try {
        if (!fs::exists(s_base_filename))
            return;

        auto file_size = fs::file_size(s_base_filename);
        if (file_size < MAX_FILE_SIZE)
            return;

        // ✅ Always base new files on the original root name
        s_file_index++;
        std::ostringstream oss;
        oss << fs::path(s_base_filename_root).stem().string() << "_" << s_file_index << ".txt";
        std::string new_file = (fs::path(s_log_dir) / oss.str()).string();

        // replace file sink
        s_file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(new_file, true);
        s_file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] %v");

        auto sinks = s_logger->sinks();
        if (!sinks.empty()) {
            sinks[0] = s_file_sink;  // index 0 = file sink
            s_logger->sinks() = sinks;
        }

        s_logger->info("Log file rotated -> {}", new_file);
        s_base_filename = new_file;
    }
    catch (const std::exception& e) {
        std::cerr << "[Log::rotate_if_needed] Error: " << e.what() << std::endl;
    }
}

void Log::log(spdlog::level::level_enum level,
              const std::string& message,
              const char* file,
              int line,
              const std::string& module,
              bool to_console)
{
    std::lock_guard<std::mutex> lock(s_mutex);

    rotate_if_needed();

    if (!s_logger)
        throw std::runtime_error("Logger not initialized. Call Log::init() first.");

    // dynamically toggle console sink
    auto& sinks = s_logger->sinks();
    auto it = std::find(sinks.begin(), sinks.end(), s_console_sink);
    if (to_console && it == sinks.end()) {
        sinks.push_back(s_console_sink);
        s_logger->sinks() = sinks;
    } else if (!to_console && it != sinks.end()) {
        sinks.erase(it);
        s_logger->sinks() = sinks;
    }

    std::string short_path = trim_project_path(file);

    std::ostringstream formatted;
    if (!module.empty())
        formatted << "[" << module << "] ";

    formatted << message << " (" << short_path << ":" << line << ")";
    s_logger->log(level, formatted.str());
}

std::string Log::trim_project_path(const std::string& full_path)
{
    if (s_project_root.empty()) return full_path;
    auto pos = full_path.find(s_project_root);
    if (pos != std::string::npos)
        return full_path.substr(pos + s_project_root.size());
    else
        return full_path;
}
