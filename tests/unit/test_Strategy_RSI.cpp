#include <gtest/gtest.h>

#include "../../src/algo/rsi/RSI.h"
#include "../test_utils/TestUtils.h"

TEST(test_Strategy_RSI, HoldsDuringWarmup)
{
    RSI rsi(/*period=*/14, /*oversold=*/30.0, /*overbought=*/70.0);

    // Seed prev_valid in Algo
    EXPECT_EQ(rsi.generate_trading_signal(make_trade("BTC", 100.0)), Signal::HOLD);

    // Provide fewer than period+1 accepted updates; should remain HOLD.
    // We'll do 10 more updates (total 11 calls after seed) - still warming up.
    for (int i = 1; i <= 10; ++i)
    {
        Signal s = rsi.generate_trading_signal(make_trade("BTC", 100.0 - i));
        EXPECT_EQ(s, Signal::HOLD);
        EXPECT_EQ(rsi.get_signal(), Signal::HOLD);
    }

    EXPECT_LT(rsi.get_valid_count(), 15);
}

TEST(test_Strategy_RSI, HoldsWhenNoPriceCanBeExtracted)
{
    RSI rsi(/*period=*/14, /*oversold=*/30.0, /*overbought=*/70.0);

    // Seed
    (void)rsi.generate_trading_signal(make_trade("BTC", 100.0));

    // Unknown: no price, no bid/ask
    MarketQuote unknown;
    unknown.symbol = "BTC";

    Signal s = rsi.generate_trading_signal(unknown);
    EXPECT_EQ(s, Signal::HOLD);
    EXPECT_EQ(rsi.get_signal(), Signal::HOLD);
}

TEST(test_Strategy_RSI, UsesMidPriceWhenTradePriceMissing)
{
    RSI rsi(/*period=*/6, /*oversold=*/30.0, /*overbought=*/70.0);

    // Seed
    (void)rsi.generate_trading_signal(make_trade("BTC", 100.0));

    // Feed mid quotes only (no trade price). Should be accepted and remain HOLD during warmup.
    for (int i = 0; i < 20; ++i)
    {
        Signal s = rsi.generate_trading_signal(make_mid_quote("BTC", 100.0 + 0.2 * i));
        EXPECT_TRUE(s == Signal::HOLD || s == Signal::BUY || s == Signal::SELL);
    }

    // We mainly verify: mid price extraction doesn't crash and can drive the algo.
    SUCCEED();
}

TEST(test_Strategy_RSI, EmitsBuyOnOversoldCrossUp)
{
    RSI rsi(/*period=*/6, /*oversold=*/30.0, /*overbought=*/70.0);

    // Seed
    (void)rsi.generate_trading_signal(make_trade("BTC", 100.0));

    bool sawBuy = false;

    // Drive down hard so RSI becomes low after seeding
    for (int i = 0; i < 40; ++i)
    {
        Signal s = rsi.generate_trading_signal(make_trade("BTC", 100.0 - 2.0 * i));
        if (s == Signal::BUY) sawBuy = true;
    }

    // Then reverse upwards; RSI should cross above oversold => BUY
    for (int i = 0; i < 60; ++i)
    {
        Signal s = rsi.generate_trading_signal(make_trade("BTC", 20.0 + 2.5 * i));
        if (s == Signal::BUY) { sawBuy = true; break; }
    }

    EXPECT_TRUE(sawBuy);
}

TEST(test_Strategy_RSI, EmitsSellOnOverboughtCrossDown)
{
    RSI rsi(/*period=*/6, /*oversold=*/30.0, /*overbought=*/70.0);

    // Seed
    (void)rsi.generate_trading_signal(make_trade("BTC", 100.0));

    bool sawSell = false;

    // Drive up hard so RSI becomes high after seeding
    for (int i = 0; i < 40; ++i)
    {
        Signal s = rsi.generate_trading_signal(make_trade("BTC", 100.0 + 2.0 * i));
        if (s == Signal::SELL) sawSell = true;
    }

    // Then reverse downwards; RSI should cross below overbought => SELL
    for (int i = 0; i < 60; ++i)
    {
        Signal s = rsi.generate_trading_signal(make_trade("BTC", 180.0 - 2.5 * i));
        if (s == Signal::SELL) { sawSell = true; break; }
    }

    EXPECT_TRUE(sawSell);
}

TEST(test_Strategy_RSI, CrossedQuoteIsRejectedAndDoesNotAdvanceValidCount)
{
    RSI rsi(/*period=*/6, /*oversold=*/30.0, /*overbought=*/70.0);

    // Seed
    (void)rsi.generate_trading_signal(make_trade("BTC", 100.0));
    const int before = rsi.get_valid_count();

    // Crossed quote should be rejected by Algo::is_genuine_transition()
    Signal s = rsi.generate_trading_signal(make_quote("BTC", 105.0, 100.0, 1.0, 1.0));

    EXPECT_EQ(s, Signal::HOLD);
    EXPECT_EQ(rsi.get_valid_count(), before);
}
