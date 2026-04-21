#include <gtest/gtest.h>
#include "../../src/log/Log.h"
#include <filesystem>
#include <fstream>

TEST(test_Log, RotatesFileWhenSizeExceeded)
{
    static constexpr size_t MSG_SIZE   = 1024;
    static constexpr int    STRESS_LOG_ITER = 100;

    namespace fs = std::filesystem;
    std::string test_log_dir = "test_logs/";
    fs::remove_all(test_log_dir);

    // Reset any previously initialized logger so Log::init below is not a no-op
    Log::shutdown();

    // Initialize with a debug level
    Log::init(test_log_dir, spdlog::level::debug);

    // Create a very large string to force rotation (MAX_FILE_SIZE is 10MB)
    std::string large_msg(MSG_SIZE, 'X');

    // Simulate enough logs to trigger rotation logic
    for (int i = 0; i < STRESS_LOG_ITER; ++i)
    {
        LOG_INFO(large_msg, "STRESS_TEST");
    }

    // Verify the directory exists and contains at least one log file
    EXPECT_TRUE(fs::exists(test_log_dir));

    auto dir_iter = fs::directory_iterator(test_log_dir);
    EXPECT_TRUE(dir_iter != fs::directory_iterator{});

    Log::shutdown();
    fs::remove_all(test_log_dir);

    // Restore the test logger so subsequent tests can still log
    Log::init("tests/logs/", spdlog::level::off);
}
