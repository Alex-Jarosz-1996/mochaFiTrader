#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../test_utils/MockTastyWorksClient.h"
#include "../test_utils/MockDX_LinkStreamer.h"
#include "../test_utils/TestUtils.h"
#include "../../src/orchestrator/Orchestrator.h"
#include "../../src/algo/macd/MACD.h"

using ::testing::Invoke;
using ::testing::Return;

TEST(test_SystemWorkflow, SimulatedMarketToOrderExecution)
{
    static constexpr int    MACD_FAST       = 2;
    static constexpr int    MACD_SLOW       = 4;
    static constexpr int    MACD_SIGNAL     = 2;
    static constexpr double ACCOUNT_BALANCE = 5000.0;
    static constexpr double TRADEABLE_AMT   = 2500.0;
    static constexpr double SEED_PRICE      = 100.0;
    static constexpr double TREND_PRICE_1   = 110.0;
    static constexpr double TREND_PRICE_2   = 120.0;
    static constexpr double TREND_PRICE_3   = 130.0;

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

    // Seed/Warmup ticks
    captured_cb(make_trade("BTC/USD", SEED_PRICE));
    captured_cb(make_trade("BTC/USD", SEED_PRICE));
    captured_cb(make_trade("BTC/USD", SEED_PRICE));
    captured_cb(make_trade("BTC/USD", SEED_PRICE));

    // Trend upwards to trigger crossover
    captured_cb(make_trade("BTC/USD", TREND_PRICE_1));
    captured_cb(make_trade("BTC/USD", TREND_PRICE_2));
    captured_cb(make_trade("BTC/USD", TREND_PRICE_3));
}
