#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../test_utils/MockTastyWorksClient.h"
#include "../test_utils/MockDX_LinkStreamer.h"
#include "../../src/orchestrator/Orchestrator.h"
#include <stdexcept>

using ::testing::_;
using ::testing::Return;
using ::testing::Throw;

TEST(test_Orchestrator, HandlesApiExceptionsGracefully)
{
    // 1. Setup Mocks
    MockTastyWorksClient mockClient;
    MockDX_LinkStreamer mockStreamer(mockClient);

    // 2. Configure Mock Client to succeed initially but fail on submission
    EXPECT_CALL(mockClient, getAccountBalance()).WillRepeatedly(Return(1000.0));
    EXPECT_CALL(mockClient, getNumberAccountPositions()).WillRepeatedly(Return(0));
    EXPECT_CALL(mockClient, getAccountNumber()).WillRepeatedly(Return("TEST-ACC"));

    // Simulate an API crash or timeout
    EXPECT_CALL(mockClient, submitOrder(_))
        .WillOnce(Throw(std::runtime_error("API Connection Lost")));

    // 3. Initialize Orchestrator
    Orchestrator orch(mockClient, mockStreamer);

    // 4. Execute and Verify
    // The test passes if on_signal does not re-throw the runtime_error.
    // In a real implementation, you would check logs or an internal error state.
    EXPECT_NO_THROW({
        orch.on_signal(Signal::BUY);
    });
}