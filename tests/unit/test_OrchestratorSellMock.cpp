#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "tests/test_utils/MockTastyWorksClient.h"
#include "tests/test_utils/MockDX_LinkStreamer.h"
#include "src/orchestrator/Orchestrator.h"

using ::testing::Return;

TEST(test_OrchestratorMock, ExecutesSellWhenPositionExists)
{
    static constexpr double OK_BALANCE   = 1000.0;
    static constexpr double TRADEABLE_AMT = 500.0;

    // 1. Setup Mocks
    MockTastyWorksClient mockClient;
    MockDX_LinkStreamer mockStreamer(mockClient);

    // 2. Configure Mock: Account has balance and 1 active position
    EXPECT_CALL(mockClient, getAccountBalance()).WillRepeatedly(Return(OK_BALANCE));
    EXPECT_CALL(mockClient, getTradeableAmount()).WillRepeatedly(Return(TRADEABLE_AMT));
    EXPECT_CALL(mockClient, getNumberAccountPositions()).WillRepeatedly(Return(1));
    EXPECT_CALL(mockClient, getAccountNumber()).WillRepeatedly(Return("TEST-ACC"));

    // 3. Initialize Orchestrator
    Orchestrator orch(mockClient, mockStreamer);

    // 4. Set Expectation: submitOrder must be called with "Sell to Close"
    EXPECT_CALL(mockClient, submitOrder(testing::_))
        .Times(1)
        .WillOnce([](const nlohmann::json& body) {
            EXPECT_EQ(body["legs"][0]["action"], "Sell to Close");
        });

    // 5. Trigger a SELL signal
    orch.on_signal(Signal::SELL);
}