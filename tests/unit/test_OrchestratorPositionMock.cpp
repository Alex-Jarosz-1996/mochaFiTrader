#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../test_utils/MockTastyWorksClient.h"
#include "../test_utils/MockDX_LinkStreamer.h"
#include "../../src/orchestrator/Orchestrator.h"

using ::testing::Return;

TEST(test_Orchestrator, PreventsDuplicateBuyWhenPositionExists)
{
    static constexpr double OK_BALANCE = 1000.0;

    // 1. Setup Mocks
    MockTastyWorksClient mockClient;
    MockDX_LinkStreamer mockStreamer(mockClient);

    // 2. Configure Mock Client: Balance is fine, but 1 position already exists
    EXPECT_CALL(mockClient, getAccountBalance())
        .WillRepeatedly(Return(OK_BALANCE));

    EXPECT_CALL(mockClient, getNumberAccountPositions())
        .WillRepeatedly(Return(1));

    EXPECT_CALL(mockClient, getAccountNumber())
        .WillRepeatedly(Return("TEST-ACC"));

    // 3. Initialize Orchestrator
    Orchestrator orch(mockClient, mockStreamer);

    // 4. Set Expectation: submitOrder should NEVER be called
    // because we already have a position.
    EXPECT_CALL(mockClient, submitOrder(testing::_)).Times(0);

    // 5. Trigger a BUY signal
    orch.on_signal(Signal::BUY);
}
