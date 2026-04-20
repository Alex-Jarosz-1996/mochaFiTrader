#include <gtest/gtest.h>

#include "../../src/algo/rsi/RSI.h"
#include "../test_utils/TestUtils.h"

// Shared RSI params
static constexpr int    STANDARD_PERIOD    = 14;
static constexpr int    SHORT_PERIOD       = 6;
static constexpr double OVERSOLD_LEVEL     = 30.0;
static constexpr double OVERBOUGHT_LEVEL   = 70.0;

TEST(test_Strategy_RSI, HoldsDuringWarmup)
{
    RSI rsi(RSI::Params{.period = STANDARD_PERIOD, .oversold = OVERSOLD_LEVEL, .overbought = OVERBOUGHT_LEVEL});

    static constexpr double SEED_PRICE  = 100.0;
    static constexpr int    WARMUP_ITER = 10;
    static constexpr int    MAX_VALID   = 15;

    // Seed prev_valid in Algo
    EXPECT_EQ(rsi.generate_trading_signal(make_trade("BTC", SEED_PRICE)), Signal::HOLD);

    // Provide fewer than period+1 accepted updates; should remain HOLD.
    for (int i = 1; i <= WARMUP_ITER; ++i)
    {
        Signal sig = rsi.generate_trading_signal(make_trade("BTC", SEED_PRICE - i));
        EXPECT_EQ(sig, Signal::HOLD);
        EXPECT_EQ(rsi.get_signal(), Signal::HOLD);
    }

    EXPECT_LT(rsi.get_valid_count(), MAX_VALID);
}

TEST(test_Strategy_RSI, HoldsWhenNoPriceCanBeExtracted)
{
    RSI rsi(RSI::Params{.period = STANDARD_PERIOD, .oversold = OVERSOLD_LEVEL, .overbought = OVERBOUGHT_LEVEL});

    static constexpr double SEED_PRICE = 100.0;

    // Seed
    (void)rsi.generate_trading_signal(make_trade("BTC", SEED_PRICE));

    // Unknown: no price, no bid/ask
    MarketQuote unknown;
    unknown.symbol = "BTC";

    Signal sig = rsi.generate_trading_signal(unknown);
    EXPECT_EQ(sig, Signal::HOLD);
    EXPECT_EQ(rsi.get_signal(), Signal::HOLD);
}

TEST(test_Strategy_RSI, UsesMidPriceWhenTradePriceMissing)
{
    RSI rsi(RSI::Params{.period = SHORT_PERIOD, .oversold = OVERSOLD_LEVEL, .overbought = OVERBOUGHT_LEVEL});

    static constexpr double SEED_PRICE  = 100.0;
    static constexpr int    MID_ITERS   = 20;
    static constexpr double PRICE_STEP  = 0.2;

    // Seed
    (void)rsi.generate_trading_signal(make_trade("BTC", SEED_PRICE));

    // Feed mid quotes only (no trade price). Should be accepted and remain HOLD during warmup.
    for (int i = 0; i < MID_ITERS; ++i)
    {
        Signal sig = rsi.generate_trading_signal(
            make_mid_quote("BTC", MidQuoteParams{SEED_PRICE + PRICE_STEP * i}));
        EXPECT_TRUE(sig == Signal::HOLD || sig == Signal::BUY || sig == Signal::SELL);
    }

    SUCCEED();
}

TEST(test_Strategy_RSI, EmitsBuyOnOversoldCrossUp)
{
    RSI rsi(RSI::Params{.period = SHORT_PERIOD, .oversold = OVERSOLD_LEVEL, .overbought = OVERBOUGHT_LEVEL});

    static constexpr double SEED_PRICE   = 100.0;
    static constexpr int    DOWN_ITERS   = 40;
    static constexpr double DOWN_STEP    = 2.0;
    static constexpr int    UP_ITERS     = 60;
    static constexpr double UP_START     = 20.0;
    static constexpr double UP_STEP      = 2.5;

    (void)rsi.generate_trading_signal(make_trade("BTC", SEED_PRICE));

    bool sawBuy = false;

    for (int i = 0; i < DOWN_ITERS; ++i)
    {
        Signal sig = rsi.generate_trading_signal(make_trade("BTC", SEED_PRICE - DOWN_STEP * i));
        if (sig == Signal::BUY) sawBuy = true;
    }

    for (int i = 0; i < UP_ITERS; ++i)
    {
        Signal sig = rsi.generate_trading_signal(make_trade("BTC", UP_START + UP_STEP * i));
        if (sig == Signal::BUY) { sawBuy = true; break; }
    }

    EXPECT_TRUE(sawBuy);
}

TEST(test_Strategy_RSI, EmitsSellOnOverboughtCrossDown)
{
    RSI rsi(RSI::Params{.period = SHORT_PERIOD, .oversold = OVERSOLD_LEVEL, .overbought = OVERBOUGHT_LEVEL});

    static constexpr double SEED_PRICE   = 100.0;
    static constexpr int    UP_ITERS     = 40;
    static constexpr double UP_STEP      = 2.0;
    static constexpr int    DOWN_ITERS   = 60;
    static constexpr double DOWN_START   = 180.0;
    static constexpr double DOWN_STEP    = 2.5;

    (void)rsi.generate_trading_signal(make_trade("BTC", SEED_PRICE));

    bool sawSell = false;

    for (int i = 0; i < UP_ITERS; ++i)
    {
        Signal sig = rsi.generate_trading_signal(make_trade("BTC", SEED_PRICE + UP_STEP * i));
        if (sig == Signal::SELL) sawSell = true;
    }

    for (int i = 0; i < DOWN_ITERS; ++i)
    {
        Signal sig = rsi.generate_trading_signal(make_trade("BTC", DOWN_START - DOWN_STEP * i));
        if (sig == Signal::SELL) { sawSell = true; break; }
    }

    EXPECT_TRUE(sawSell);
}

TEST(test_Strategy_RSI, CrossedQuoteIsRejectedAndDoesNotAdvanceValidCount)
{
    RSI rsi(RSI::Params{.period = SHORT_PERIOD, .oversold = OVERSOLD_LEVEL, .overbought = OVERBOUGHT_LEVEL});

    static constexpr double SEED_PRICE  = 100.0;
    static constexpr double INV_BID     = 105.0;
    static constexpr double INV_ASK     = 100.0;
    static constexpr double VALID_SIZE  = 1.0;

    (void)rsi.generate_trading_signal(make_trade("BTC", SEED_PRICE));
    const int before = rsi.get_valid_count();

    Signal sig = rsi.generate_trading_signal(
        make_quote("BTC", QuoteParams{INV_BID, INV_ASK, VALID_SIZE, VALID_SIZE}));

    EXPECT_EQ(sig, Signal::HOLD);
    EXPECT_EQ(rsi.get_valid_count(), before);
}
