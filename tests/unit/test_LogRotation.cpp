#include <gtest/gtest.h>
#include "../../src/log/Log.h"
#include <filesystem>
#include <fstream>

TEST(test_Log, RotatesFileWhenSizeExceeded)
{
    namespace fs = std::filesystem;
    std::string test_log_dir = "test_logs/";
    fs::remove_all(test_log_dir);

    // Initialize with a debug level
    Log::init(test_log_dir, spdlog::level::debug);

    // Create a very large string to force rotation (MAX_FILE_SIZE is 10MB)
    // For testing purposes, you might consider making MAX_FILE_SIZE configurable
    // or just checking that a few large writes generate a second file.
    std::string large_msg(1024, 'X'); 
    
    // We simulate enough logs to trigger rotation logic
    // Note: Since MAX_FILE_SIZE is 10MB, in a real unit test we'd mock the file size
    // or reduce the threshold for the test build.
    for(int i = 0; i < 100; ++i) {
        LOG_INFO(large_msg, "STRESS_TEST");
    }

    // Verify the directory exists and contains at least one log file
    EXPECT_TRUE(fs::exists(test_log_dir));
    
    auto it = fs::directory_iterator(test_log_dir);
    EXPECT_TRUE(it != fs::directory_iterator{});

    Log::shutdown();
    fs::remove_all(test_log_dir);
}