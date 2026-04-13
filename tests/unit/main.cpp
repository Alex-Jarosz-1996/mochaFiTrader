#include <gtest/gtest.h>
#include "../../src/log/Log.h"

int main(int argc, char** argv)
{
    Log::init("tests/logs/", spdlog::level::off); // Keep tests quiet but initialized
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    Log::shutdown();
    return result;
}
