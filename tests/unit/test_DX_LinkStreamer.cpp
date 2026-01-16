#include <gtest/gtest.h>
#include <memory>

#include "../../src/log/Log.h"
#include "../../src/streamer/DX_LinkStreamer.h"

class test_DX_LinkStreamer : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Log::init("src/log/");
    }
};

TEST_F(test_DX_LinkStreamer, CanConstructStreamer)
{
    std::unique_ptr<TastyWorksClient> twClient = std::make_unique<TastyWorksClient>();
    
    EXPECT_NO_THROW({
        std::unique_ptr<DX_LinkStreamer> dxlStreamer = std::make_unique<DX_LinkStreamer>(
            *twClient
        );
    });
}

TEST_F(test_DX_LinkStreamer, CanStream)
{
    std::unique_ptr<TastyWorksClient> twClient = std::make_unique<TastyWorksClient>();
    std::unique_ptr<DX_LinkStreamer> dxlStreamer = std::make_unique<DX_LinkStreamer>(
        *twClient
    );

    bool called = false;
    dxlStreamer->set_on_quote([&](const MarketQuote& quote)
    {
        called = true;
    });

    SUCCEED();
}
