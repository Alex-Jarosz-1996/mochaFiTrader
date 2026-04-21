#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../test_utils/MockTastyWorksClient.h"
#include "../test_utils/MockDX_LinkStreamer.h"
#include "../../src/orchestrator/Orchestrator.h"

using ::testing::Return;

// ACCOUNT_MIN = 100.0 as defined in TastyWorks.h
static constexpr double ACCOUNT_MIN     = 100.0;
static constexpr double JUST_BELOW_MIN  = ACCOUNT_MIN - 0.01;
static constexpr double AT_MIN          = ACCOUNT_MIN;
static constexpr double ABOVE_MIN       = ACCOUNT_MIN + 1.0;
static constexpr double NEGATIVE_BALANCE = -50.0;

static auto make_orch_with_balance(MockTastyWorksClient& mockClient,
                                   MockDX_LinkStreamer&   mockStreamer,
                                   double balance) -> Orchestrator
{
    EXPECT_CALL(mockClient, getAccountBalance()).WillRepeatedly(Return(balance));
    EXPECT_CALL(mockClient, getAccountNumber()).WillRepeatedly(Return("TEST-ACC"));
    return {mockClient, mockStreamer};
}

// --- Throws when balance is below the minimum ---

TEST(test_OrchestratorMock, BlocksTradeWhenBalanceIsLow)
{
    MockTastyWorksClient mockClient;
    MockDX_LinkStreamer  mockStreamer(mockClient);
    EXPECT_CALL(mockClient, submitOrder(testing::_)).Times(0);

    Orchestrator orch = make_orch_with_balance(mockClient, mockStreamer, JUST_BELOW_MIN);

    EXPECT_THROW(orch.on_signal(Signal::BUY), std::invalid_argument);
}

TEST(test_OrchestratorMock, ThrowsWhenBalanceIsZero)
{
    MockTastyWorksClient mockClient;
    MockDX_LinkStreamer  mockStreamer(mockClient);
    EXPECT_CALL(mockClient, submitOrder(testing::_)).Times(0);

    Orchestrator orch = make_orch_with_balance(mockClient, mockStreamer, 0.0);

    EXPECT_THROW(orch.on_signal(Signal::BUY), std::invalid_argument);
}

TEST(test_OrchestratorMock, ThrowsWhenBalanceIsNegative)
{
    MockTastyWorksClient mockClient;
    MockDX_LinkStreamer  mockStreamer(mockClient);
    EXPECT_CALL(mockClient, submitOrder(testing::_)).Times(0);

    Orchestrator orch = make_orch_with_balance(mockClient, mockStreamer, NEGATIVE_BALANCE);

    EXPECT_THROW(orch.on_signal(Signal::BUY), std::invalid_argument);
}

TEST(test_OrchestratorMock, ThrowsOnSellSignalWhenBalanceBelowMinimum)
{
    MockTastyWorksClient mockClient;
    MockDX_LinkStreamer  mockStreamer(mockClient);
    EXPECT_CALL(mockClient, submitOrder(testing::_)).Times(0);

    Orchestrator orch = make_orch_with_balance(mockClient, mockStreamer, JUST_BELOW_MIN);

    EXPECT_THROW(orch.on_signal(Signal::SELL), std::invalid_argument);
}

TEST(test_OrchestratorMock, ThrowsOnHoldSignalWhenBalanceBelowMinimum)
{
    MockTastyWorksClient mockClient;
    MockDX_LinkStreamer  mockStreamer(mockClient);

    Orchestrator orch = make_orch_with_balance(mockClient, mockStreamer, JUST_BELOW_MIN);

    // HOLD is checked after the balance guard, so the throw still fires first
    EXPECT_THROW(orch.on_signal(Signal::HOLD), std::invalid_argument);
}

// --- Does NOT throw when balance meets or exceeds the minimum ---

TEST(test_OrchestratorMock, DoesNotThrowWhenBalanceIsExactlyAtMinimum)
{
    MockTastyWorksClient mockClient;
    MockDX_LinkStreamer  mockStreamer(mockClient);
    EXPECT_CALL(mockClient, getNumberAccountPositions()).WillRepeatedly(Return(0));
    EXPECT_CALL(mockClient, getTradeableAmount()).WillRepeatedly(Return(AT_MIN));
    EXPECT_CALL(mockClient, submitOrder(testing::_)).Times(1);

    Orchestrator orch = make_orch_with_balance(mockClient, mockStreamer, AT_MIN);

    EXPECT_NO_THROW(orch.on_signal(Signal::BUY));
}

TEST(test_OrchestratorMock, DoesNotThrowWhenBalanceIsAboveMinimum)
{
    MockTastyWorksClient mockClient;
    MockDX_LinkStreamer  mockStreamer(mockClient);
    EXPECT_CALL(mockClient, getNumberAccountPositions()).WillRepeatedly(Return(0));
    EXPECT_CALL(mockClient, getTradeableAmount()).WillRepeatedly(Return(ABOVE_MIN));
    EXPECT_CALL(mockClient, submitOrder(testing::_)).Times(1);

    Orchestrator orch = make_orch_with_balance(mockClient, mockStreamer, ABOVE_MIN);

    EXPECT_NO_THROW(orch.on_signal(Signal::BUY));
}
