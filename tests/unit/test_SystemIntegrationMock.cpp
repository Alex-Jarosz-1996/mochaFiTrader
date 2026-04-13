#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../test_utils/MockTastyWorksClient.h"
#include "../test_utils/MockDX_LinkStreamer.h"
#include "../test_utils/TestUtils.h"
#include "../../src/orchestrator/Orchestrator.h"
#include "../../src/algo/macd/MACD.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

TEST(test_SystemWorkflow, SimulatedMarketToOrderExecution)
{
    // 1. Setup Mocks
    MockTastyWorksClient mockClient;
    MockDX_LinkStreamer mockStreamer(mockClient);
    
    // 2. Setup Real Strategy (We test the real math here)
    // Using small periods for fast triggering
    auto strategy = std::make_shared<MACD>(2, 4, 2);

    // 3. Setup Orchestrator
    Orchestrator orch(mockClient, mockStreamer);

    // 4. Set Expectations
    // We expect the streamer to receive a callback function
    DX_LinkStreamer::QuoteCallback captured_cb;
    EXPECT_CALL(mockStreamer, set_on_quote(_))
        .WillOnce(Invoke([&](DX_LinkStreamer::QuoteCallback qcb) {
            captured_cb = qcb;
        }));

    EXPECT_CALL(mockClient, getAccountNumber()).WillRepeatedly(Return("TEST-123"));
    EXPECT_CALL(mockClient, getAccountBalance()).WillRepeatedly(Return(5000.0));
    EXPECT_CALL(mockClient, getTradeableAmount()).WillRepeatedly(Return(2500.0));
    EXPECT_CALL(mockClient, getNumberAccountPositions()).WillRepeatedly(Return(0));

    // This is the "Payoff": we expect an order to be submitted eventually
    EXPECT_CALL(mockClient, submitOrder(_)).Times(1);

    // 5. Execute
    // Simulate the Orchestrator registering with the streamer
    mockStreamer.set_on_quote([&](const MarketQuote& q) {
        Signal sig = strategy->generate_trading_signal(q);
        orch.on_signal(sig);
    });

    // Simulate a price trend that triggers a BUY signal
    // Seed/Warmup ticks
    captured_cb(make_trade("BTC/USD", 100.0));
    captured_cb(make_trade("BTC/USD", 100.0));
    captured_cb(make_trade("BTC/USD", 100.0));
    captured_cb(make_trade("BTC/USD", 100.0));
    
    // Trend upwards to trigger crossover
    captured_cb(make_trade("BTC/USD", 110.0));
    captured_cb(make_trade("BTC/USD", 120.0));
    captured_cb(make_trade("BTC/USD", 130.0));
}