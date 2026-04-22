#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "tests/test_utils/MockTastyWorksClient.h"
#include "tests/test_utils/MockDX_LinkStreamer.h"
#include "tests/test_utils/TestUtils.h"
#include "src/orchestrator/Orchestrator.h"
#include "src/algo/macd/MACD.h"

using ::testing::Invoke;
using ::testing::Return;

TEST(test_SystemWorkflow, SimulatedMarketToOrderExecution)
{
    static constexpr int    MACD_FAST         = 2;
    static constexpr int    MACD_SLOW         = 4;
    static constexpr int    MACD_SIGNAL       = 2;
    // trigger_window = 3 * MACD_SLOW = 12; need 1 seed + 12 valid ticks to clear it
    static constexpr int    WARMUP_TICKS      = 13;
    static constexpr double ACCOUNT_BALANCE   = 5000.0;
    static constexpr double TRADEABLE_AMT     = 2500.0;
    static constexpr double SEED_PRICE        = 100.0;
    // Drop 14% (within the 15% jump-rejection threshold) to push MACD below signal line
    static constexpr double DOWNTREND_PRICE   = 86.0;
    // Recover ~11.6% — fast EMA rebounds ahead of slow EMA, crossing MACD above signal → BUY
    static constexpr double RECOVERY_PRICE    = 96.0;

    // 1. Setup Mocks
    MockTastyWorksClient mockClient;
    MockDX_LinkStreamer mockStreamer(mockClient);

    // 2. Setup Real Strategy (We test the real math here)
    auto strategy = std::make_shared<MACD>(
        MACD::Params{.fast = MACD_FAST, .slow = MACD_SLOW, .signal = MACD_SIGNAL});

    // 3. Setup Orchestrator
    Orchestrator orch(mockClient, mockStreamer);

    // 4. Set Expectations
    DX_LinkStreamer::QuoteCallback captured_cb;
    EXPECT_CALL(mockStreamer, set_on_quote(testing::_))
        .WillOnce(Invoke([&](DX_LinkStreamer::QuoteCallback qcb) {
            captured_cb = std::move(qcb);
        }));

    EXPECT_CALL(mockClient, getAccountNumber()).WillRepeatedly(Return("TEST-123"));
    EXPECT_CALL(mockClient, getAccountBalance()).WillRepeatedly(Return(ACCOUNT_BALANCE));
    EXPECT_CALL(mockClient, getTradeableAmount()).WillRepeatedly(Return(TRADEABLE_AMT));
    EXPECT_CALL(mockClient, getNumberAccountPositions()).WillRepeatedly(Return(0));

    // This is the "Payoff": we expect an order to be submitted eventually
    EXPECT_CALL(mockClient, submitOrder(testing::_)).Times(1);

    // 5. Execute
    mockStreamer.set_on_quote([&](const MarketQuote& quote) {
        Signal sig = strategy->generate_trading_signal(quote);
        orch.on_signal(sig);
    });

    // Seed + warmup: 1 seed tick then WARMUP_TICKS-1 flat ticks to reach valid_count == 12
    for (int i = 0; i < WARMUP_TICKS; ++i)
        captured_cb(make_trade("BTC/USD", SEED_PRICE));

    // Downtrend: pushes MACD negative and below the signal line
    captured_cb(make_trade("BTC/USD", DOWNTREND_PRICE));
    captured_cb(make_trade("BTC/USD", DOWNTREND_PRICE));

    // Recovery: fast EMA rebounds faster than slow EMA → MACD crosses above signal → BUY
    captured_cb(make_trade("BTC/USD", RECOVERY_PRICE));
}
