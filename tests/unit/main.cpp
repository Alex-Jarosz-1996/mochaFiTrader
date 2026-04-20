#include <gtest/gtest.h>
#include "../../src/log/Log.h"

auto main(int argc, char** argv) -> int
{
    Log::init("tests/logs/", spdlog::level::off); // Keep tests quiet but initialized
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    Log::shutdown();
    return result;
}
