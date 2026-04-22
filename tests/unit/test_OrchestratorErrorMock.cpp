#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "tests/test_utils/MockTastyWorksClient.h"
#include "tests/test_utils/MockDX_LinkStreamer.h"
#include "src/orchestrator/Orchestrator.h"
#include <stdexcept>

using ::testing::Return;
using ::testing::Throw;

TEST(test_OrchestratorMock, HandlesApiExceptionsGracefully)
{
    static constexpr double OK_BALANCE = 1000.0;

    // 1. Setup Mocks
    MockTastyWorksClient mockClient;
    MockDX_LinkStreamer mockStreamer(mockClient);

    // 2. Configure Mock Client to succeed initially but fail on submission
    EXPECT_CALL(mockClient, getAccountBalance()).WillRepeatedly(Return(OK_BALANCE));
    EXPECT_CALL(mockClient, getNumberAccountPositions()).WillRepeatedly(Return(0));
    EXPECT_CALL(mockClient, getAccountNumber()).WillRepeatedly(Return("TEST-ACC"));

    // Simulate an API crash or timeout
    EXPECT_CALL(mockClient, submitOrder(testing::_))
        .WillOnce(Throw(std::runtime_error("API Connection Lost")));

    // 3. Initialize Orchestrator
    Orchestrator orch(mockClient, mockStreamer);

    // 4. Execute and Verify
    EXPECT_NO_THROW({
        orch.on_signal(Signal::BUY);
    });
}
