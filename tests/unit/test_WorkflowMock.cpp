#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../test_utils/MockTastyWorksClient.h"
#include "../test_utils/TestUtils.h"
#include "../../src/orchestrator/Orchestrator.h"
#include "../../src/streamer/DX_LinkStreamer.h"

using ::testing::_;
using ::testing::Return;
using ::testing::AtLeast;

TEST(test_Workflow, FullTradingLoopMock)
{
    // 1. Setup Mocks
    MockTastyWorksClient mockClient;
    
    // Define expected behavior for the mock
    EXPECT_CALL(mockClient, getAccountNumber())
        .WillRepeatedly(Return("555-TEST-ACCOUNT"));
    
    EXPECT_CALL(mockClient, getAccountBalance())
        .WillRepeatedly(Return(10000.0));

    // 2. Setup Orchestrator (Injecting the mock)
    // Note: Since DX_LinkStreamer isn't virtual yet, we pass the mockClient to it too
    DX_LinkStreamer streamer(mockClient); 
    Orchestrator orch(mockClient, streamer);

    // 3. Define the critical expectation: An order MUST be submitted when we signal BUY
    // We verify that the JSON passed to submitOrder contains "Buy to Open"
    EXPECT_CALL(mockClient, submitOrder(_))
        .Times(1)
        .WillOnce([](const nlohmann::json& body) {
            EXPECT_EQ(body["legs"][0]["action"], "Buy to Open");
            EXPECT_EQ(body["legs"][0]["symbol"], "BTC/USD");
        });

    // 4. Simulate the workflow: Orchestrator receives a BUY signal
    // In the real app, this comes from the streamer -> strategy -> orchestrator
    orch.on_signal(Signal::BUY);

    // The test will fail here if mockClient.submitOrder was not called as specified.
}