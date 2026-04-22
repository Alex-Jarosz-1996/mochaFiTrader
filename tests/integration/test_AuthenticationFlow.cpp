#include <gtest/gtest.h>
#include "src/tastyworks/TastyWorks.h"
#include "src/log/Log.h"
#include "src/config/Config.h"

class AuthenticationIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        Log::init("src/log/", spdlog::level::debug);
    }
};

TEST_F(AuthenticationIntegrationTest, FullHandshakeStepByStep) // NOLINT(readability-function-cognitive-complexity)
{
    std::unique_ptr<TastyWorksClient> client;
    try {
        client = std::make_unique<TastyWorksClient>();
    } catch (const std::exception& e) {
        GTEST_SKIP() << "Skipping: Authentication handshake failed (check credentials/internet). Reason: " << e.what();
    }

    // Step 1: Config loading
    EXPECT_NO_THROW(client->getBaseUrl());
    ASSERT_FALSE(client->getAccountNumber().empty()) << "Account number missing in config.json";

    // Step 2: Verify session token and stream URL were populated
    ASSERT_FALSE(client->getAPI_QuoteToken().empty()) << "Failed to retrieve API Quote Token";
    ASSERT_FALSE(client->getDX_LinkUrl().empty()) << "Failed to retrieve DXLink URL";

    LOG_INFO("Integration: Authentication handshake successful.", "TEST");
}

TEST_F(AuthenticationIntegrationTest, VerifyAccountBalanceAccess) {
    try {
        TastyWorksClient client;
        double balance = client.getAccountBalance();
        EXPECT_GT(balance, -1000000.0); // Simple sanity check that we got a numeric response
        
        double tradeable = client.getTradeableAmount();
        EXPECT_LE(tradeable, balance);
    } catch (const std::exception& e) {
        GTEST_SKIP() << "Skipping balance test: Auth failed.";
    }
}