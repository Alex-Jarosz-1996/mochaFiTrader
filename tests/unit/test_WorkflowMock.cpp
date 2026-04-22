#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "tests/test_utils/MockTastyWorksClient.h"
#include "tests/test_utils/TestUtils.h"
#include "src/orchestrator/Orchestrator.h"
#include "src/streamer/DX_LinkStreamer.h"

using ::testing::Return;
using ::testing::AtLeast;

TEST(test_Workflow, FullTradingLoopMock)
{
    static constexpr double ACCOUNT_BALANCE = 10000.0;

    // 1. Setup Mocks
    MockTastyWorksClient mockClient;

    // Define expected behavior for the mock
    EXPECT_CALL(mockClient, getDX_LinkUrl()).WillRepeatedly(Return("wss://fake.url"));
    EXPECT_CALL(mockClient, getAPI_QuoteToken()).WillRepeatedly(Return("fake-token"));
    EXPECT_CALL(mockClient, getAccountNumber()).WillRepeatedly(Return("555-TEST-ACCOUNT"));
    EXPECT_CALL(mockClient, getAccountBalance()).WillRepeatedly(Return(ACCOUNT_BALANCE));
    EXPECT_CALL(mockClient, getNumberAccountPositions()).WillRepeatedly(Return(0));
    EXPECT_CALL(mockClient, getTradeableAmount()).WillRepeatedly(Return(ACCOUNT_BALANCE));

    // 2. Setup Orchestrator (Injecting the mock)
    DX_LinkStreamer streamer(mockClient);
    Orchestrator orch(mockClient, streamer);

    // 3. Define the critical expectation: An order MUST be submitted when we signal BUY
    EXPECT_CALL(mockClient, submitOrder(testing::_))
        .Times(1)
        .WillOnce([](const nlohmann::json& body) {
            EXPECT_EQ(body["legs"][0]["action"], "Buy to Open");
            EXPECT_EQ(body["legs"][0]["symbol"], "BTC/USD:CXTALP");
        });

    // 4. Simulate the workflow: Orchestrator receives a BUY signal
    // In the real app, this comes from the streamer -> strategy -> orchestrator
    orch.on_signal(Signal::BUY);

    // The test will fail here if mockClient.submitOrder was not called as specified.
}