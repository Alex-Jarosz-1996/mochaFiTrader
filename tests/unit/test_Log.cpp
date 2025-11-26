#include <gtest/gtest.h>
#include "../../src/log/Log.h"

TEST(LogTest, CanInitializeLogger)
{
    EXPECT_NO_THROW({
        Log::init("src/log/");
    });

    // Basic smoke test logging
    EXPECT_NO_THROW({
        LOG_INFO("Test info log", "LOG_H");
        LOG_DEBUG("Test debug log", "LOG_H");
        LOG_WARN("Test warn log", "LOG_H");
        LOG_ERROR("Test error log", "LOG_H");
        LOG_FILE_ONLY("File-only log", "LOG_H");
    });
}
