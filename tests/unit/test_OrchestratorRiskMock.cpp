#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../test_utils/MockTastyWorksClient.h"
#include "../test_utils/MockDX_LinkStreamer.h"
#include "../../src/orchestrator/Orchestrator.h"

using ::testing::Return;

TEST(test_Orchestrator, BlocksTradeWhenBalanceIsLow)
{
    static constexpr double LOW_BALANCE = 50.0;

    // 1. Setup Mocks
    MockTastyWorksClient mockClient;
    MockDX_LinkStreamer mockStreamer(mockClient);

    // 2. Configure Mock Client to return a balance below the threshold
    // Assuming ACCOUNT_MIN is 100 as per TastyWorks.h
    EXPECT_CALL(mockClient, getAccountBalance())
        .WillRepeatedly(Return(LOW_BALANCE));

    EXPECT_CALL(mockClient, getAccountNumber())
        .WillRepeatedly(Return("TEST-ACC"));

    // 3. Initialize Orchestrator
    Orchestrator orch(mockClient, mockStreamer);

    // 4. Set Expectation: submitOrder should NEVER be called
    EXPECT_CALL(mockClient, submitOrder(testing::_)).Times(0);

    // 5. Trigger a BUY signal
    // Even though the signal is BUY, the Orchestrator should check the balance
    // and decide not to execute.
    orch.on_signal(Signal::BUY);
}
