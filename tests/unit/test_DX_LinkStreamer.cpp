#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>

#include "../../src/log/Log.h"
#include "../../src/streamer/DX_LinkStreamer.h"
#include "../test_utils/MockTastyWorksClient.h"

using ::testing::Return;

class test_DX_LinkStreamer : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Log::init("src/log/");
    }
};

// --- Mock-based unit tests (no network required) ---

TEST_F(test_DX_LinkStreamer, ConstructsWithMockedCredentials)
{
    MockTastyWorksClient mockClient;
    EXPECT_CALL(mockClient, getDX_LinkUrl()).WillOnce(Return("wss://fake.url"));
    EXPECT_CALL(mockClient, getAPI_QuoteToken()).WillOnce(Return("fake-token"));

    EXPECT_NO_THROW({
        DX_LinkStreamer streamer(mockClient);
    });
}

TEST_F(test_DX_LinkStreamer, SetsQuoteCallbackWithoutThrowing)
{
    MockTastyWorksClient mockClient;
    EXPECT_CALL(mockClient, getDX_LinkUrl()).WillOnce(Return("wss://fake.url"));
    EXPECT_CALL(mockClient, getAPI_QuoteToken()).WillOnce(Return("fake-token"));

    DX_LinkStreamer streamer(mockClient);
    bool called = false;

    EXPECT_NO_THROW({
        streamer.set_on_quote([&](const MarketQuote&) { called = true; });
    });
}

TEST_F(test_DX_LinkStreamer, AuthorizeMessageContainsInjectedToken)
{
    MockTastyWorksClient mockClient;
    EXPECT_CALL(mockClient, getDX_LinkUrl()).WillOnce(Return("wss://fake.url"));
    EXPECT_CALL(mockClient, getAPI_QuoteToken()).WillOnce(Return("my-token-123"));

    DX_LinkStreamer streamer(mockClient);

    const auto& auth = streamer.get_authorize_msg();
    EXPECT_EQ(auth["type"].get<std::string>(), "AUTH");
    EXPECT_EQ(auth["token"].get<std::string>(), "my-token-123");
}

TEST_F(test_DX_LinkStreamer, AuthorizeMessageUsesSetupChannel)
{
    MockTastyWorksClient mockClient;
    EXPECT_CALL(mockClient, getDX_LinkUrl()).WillOnce(Return("wss://fake.url"));
    EXPECT_CALL(mockClient, getAPI_QuoteToken()).WillOnce(Return("tok"));

    DX_LinkStreamer streamer(mockClient);

    EXPECT_EQ(streamer.get_authorize_msg()["channel"].get<int>(), 0);
}

// --- Live tests (skipped if credentials are absent) ---

struct DX_LinkFixture
{
    std::unique_ptr<TastyWorksClient> twClient;
    std::unique_ptr<DX_LinkStreamer>  streamer;

    auto try_build(std::string& skip_reason) -> bool
    {
        try
        {
            twClient = std::make_unique<TastyWorksClient>();
            streamer = std::make_unique<DX_LinkStreamer>(*twClient);
            return true;
        }
        catch (const std::exception& e)
        {
            skip_reason = std::string("unable to construct: ") + e.what();
            return false;
        }
    }
};

TEST_F(test_DX_LinkStreamer, Live_CanConstructStreamer)
{
    DX_LinkFixture fixture;
    std::string skip_reason;
    if (!fixture.try_build(skip_reason)) GTEST_SKIP() << skip_reason;

    SUCCEED();
}

TEST_F(test_DX_LinkStreamer, Live_CanSetQuoteCallback)
{
    DX_LinkFixture fixture;
    std::string skip_reason;
    if (!fixture.try_build(skip_reason)) GTEST_SKIP() << skip_reason;

    bool called = false;
    EXPECT_NO_THROW({
        fixture.streamer->set_on_quote([&](const MarketQuote&) { called = true; });
    });
}
