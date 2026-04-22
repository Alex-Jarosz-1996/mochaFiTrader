#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include "src/tastyworks/TastyWorks.h"
#include "src/log/Log.h"
#include "tests/test_utils/MockTastyWorksClient.h"

using ::testing::Return;

class test_TastyWorks : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Log::init("src/log/");
    }
};

// --- Mock-based unit tests (no network required) ---

TEST_F(test_TastyWorks, MockClient_ReturnsConfiguredToken)
{
    MockTastyWorksClient mockClient;
    EXPECT_CALL(mockClient, getAPI_QuoteToken()).WillOnce(Return("tok-abc"));
    EXPECT_EQ(mockClient.getAPI_QuoteToken(), "tok-abc");
}

TEST_F(test_TastyWorks, MockClient_ReturnsConfiguredDXUrl)
{
    MockTastyWorksClient mockClient;
    EXPECT_CALL(mockClient, getDX_LinkUrl()).WillOnce(Return("wss://stream.test"));
    EXPECT_EQ(mockClient.getDX_LinkUrl(), "wss://stream.test");
}

TEST_F(test_TastyWorks, MockClient_ReturnsConfiguredAccountBalance)
{
    static constexpr double EXPECTED_BALANCE = 1234.56;

    MockTastyWorksClient mockClient;
    EXPECT_CALL(mockClient, getAccountBalance()).WillOnce(Return(EXPECTED_BALANCE));
    EXPECT_DOUBLE_EQ(mockClient.getAccountBalance(), EXPECTED_BALANCE);
}

TEST_F(test_TastyWorks, MockClient_ReturnsConfiguredPositionCount)
{
    MockTastyWorksClient mockClient;
    EXPECT_CALL(mockClient, getNumberAccountPositions()).WillOnce(Return(3));
    EXPECT_EQ(mockClient.getNumberAccountPositions(), 3U);
}

// --- Live test (skipped if credentials are absent) ---

struct TastyWorksFixture
{
    std::unique_ptr<TastyWorksClient> twClient;

    auto try_build(std::string& skip_reason) -> bool
    {
        try
        {
            twClient = std::make_unique<TastyWorksClient>();
            return true;
        }
        catch (const std::exception& e)
        {
            skip_reason = std::string("unable to construct: ") + e.what();
            return false;
        }
    }
};

TEST_F(test_TastyWorks, Live_HandlesTokenGeneration)
{
    TastyWorksFixture fixture;
    std::string skip_reason;
    if (!fixture.try_build(skip_reason)) GTEST_SKIP() << skip_reason;

    EXPECT_FALSE(fixture.twClient->_session_token.empty());
    EXPECT_FALSE(fixture.twClient->_api_quote_token.empty());
    EXPECT_FALSE(fixture.twClient->_dx_link_url.empty());
}
