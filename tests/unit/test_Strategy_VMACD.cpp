#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <vector>

#include "../../src/algo/vmacd/VMACD.h"
#include "../test_utils/TestUtils.h"

TEST(test_Strategy_VMACD, HoldsUntilBothPriceAndImbalanceInitialised)
{
    VMACD vmacd(/*fast=*/3, /*slow=*/6, /*signal=*/3);

    // First call seeds prev_valid (Algo behavior) => no processing yet
    EXPECT_EQ(vmacd.generate_trading_signal(make_trade("BTC", 100.0)), Signal::HOLD);

    // Feed only trades for a while: price updates, but imbalance never initialises => HOLD forever
    bool sawNonHold = false;
    for (int i = 0; i < 100; ++i)
    {
        Signal s = vmacd.generate_trading_signal(make_trade("BTC", 100.0 + 0.1 * i));
        if (s != Signal::HOLD) sawNonHold = true;
    }
    EXPECT_FALSE(sawNonHold);

    // Now start feeding quotes (imbalance) too; still may be in warmup, but should not crash.
    for (int i = 0; i < 10; ++i)
    {
        Signal s = vmacd.generate_trading_signal(make_quote_with_imbalance("BTC", 100.0, 10.0, 1.0, 0.7));
        EXPECT_TRUE(s == Signal::BUY || s == Signal::SELL || s == Signal::HOLD);
    }
}

TEST(test_Strategy_VMACD, ProducesBuyAndSellOnRegimeSwings)
{
    VMACD vmacd(/*fast=*/3, /*slow=*/6, /*signal=*/3);

    // Seed
    (void)vmacd.generate_trading_signal(make_trade("BTC", 100.0));

    bool sawBuy = false;
    bool sawSell = false;

    // We will interleave quote and trade packets.
    // Use strong imbalance regimes (+/-0.9) so the imbalance MACD tends to have meaningful magnitude
    // and exceeds imb_strength(0.12) more often.

    // Part A: bearish imbalance + downtrend
    for (int i = 0; i < 80; ++i)
    {
        (void)vmacd.generate_trading_signal(make_quote_with_imbalance("BTC", 100.0, 10.0, 1.0, -0.9));
        Signal s = vmacd.generate_trading_signal(make_trade("BTC", 100.0 - 0.4 * i));
        sawBuy  |= (s == Signal::BUY);
        sawSell |= (s == Signal::SELL);
    }

    // Part B: bullish imbalance + uptrend
    for (int i = 0; i < 120; ++i)
    {
        (void)vmacd.generate_trading_signal(make_quote_with_imbalance("BTC", 100.0, 10.0, 1.0, +0.9));
        Signal s = vmacd.generate_trading_signal(make_trade("BTC", 68.0 + 0.6 * i));
        sawBuy  |= (s == Signal::BUY);
        sawSell |= (s == Signal::SELL);
    }

    // Part C: bearish imbalance + downtrend again (helps force a SELL if not seen yet)
    for (int i = 0; i < 120; ++i)
    {
        (void)vmacd.generate_trading_signal(make_quote_with_imbalance("BTC", 100.0, 10.0, 1.0, -0.9));
        Signal s = vmacd.generate_trading_signal(make_trade("BTC", 140.0 - 0.7 * i));
        sawBuy  |= (s == Signal::BUY);
        sawSell |= (s == Signal::SELL);
    }

    EXPECT_TRUE(sawBuy);
    EXPECT_TRUE(sawSell);
}

TEST(test_Strategy_VMACD, IgnoresInvalidImbalanceInputsAndDoesNotCrash)
{
    VMACD vmacd(/*fast=*/3, /*slow=*/6, /*signal=*/3);

    // Seed prev_valid
    (void)vmacd.generate_trading_signal(make_trade("BTC", 100.0));

    // Valid quote/trade to get things started
    EXPECT_EQ(vmacd.generate_trading_signal(make_quote("BTC", 99.0, 101.0, 1.0, 1.0)), Signal::HOLD);
    EXPECT_EQ(vmacd.generate_trading_signal(make_trade("BTC", 100.0)), Signal::HOLD);

    // Invalid quote: missing sizes
    MarketQuote missing_sizes;
    missing_sizes.symbol = "BTC";
    missing_sizes.bidPrice = 99.0;
    missing_sizes.askPrice = 101.0;

    Signal s1 = vmacd.generate_trading_signal(missing_sizes);
    EXPECT_TRUE(s1 == Signal::BUY || s1 == Signal::SELL || s1 == Signal::HOLD);

    // Invalid quote: denom=0
    Signal s2 = vmacd.generate_trading_signal(make_quote("BTC", 99.0, 101.0, 0.0, 0.0));
    EXPECT_TRUE(s2 == Signal::BUY || s2 == Signal::SELL || s2 == Signal::HOLD);

    // Invalid quote: crossed market (bid > ask) should be rejected by Algo::is_genuine_transition()
    Signal s3 = vmacd.generate_trading_signal(make_quote("BTC", 105.0, 100.0, 1.0, 1.0));
    EXPECT_EQ(s3, Signal::HOLD);
}
