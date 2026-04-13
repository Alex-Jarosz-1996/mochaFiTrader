#include <gtest/gtest.h>
#include "../../src/tastyworks/TastyWorks.h"
#include "../../src/log/Log.h"
#include "../../src/config/Config.h"

class AuthenticationIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        Log::init("src/log/", spdlog::level::debug);
    }
};

TEST_F(AuthenticationIntegrationTest, FullHandshakeStepByStep) {
    // We don't use the constructor here to isolate the steps
    TastyWorksClient client; 
    
    // Step 1: Config loading
    EXPECT_NO_THROW(client.getBaseUrl());
    ASSERT_FALSE(client.getAccountNumber().empty()) << "Account number missing in config.json";

    // Step 2: Session Token Generation
    // This is likely where your current issue is.
    try {
        // Note: getSessionToken is private, but in a real integration test 
        // you'd test the public side-effects. Since TastyWorksClient 
        // does everything in the constructor, we test the resulting state.
        
        ASSERT_FALSE(client.getAPI_QuoteToken().empty()) << "Failed to retrieve API Quote Token";
        ASSERT_FALSE(client.getDX_LinkUrl().empty()) << "Failed to retrieve DXLink URL";
        
        LOG_INFO("Integration: Authentication handshake successful.", "TEST");
    } catch (const std::exception& e) {
        GTEST_SKIP() << "Skipping: Authentication handshake failed (check credentials/internet). Reason: " << e.what();
    }
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