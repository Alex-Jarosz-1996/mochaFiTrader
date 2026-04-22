#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <vector>

#include "src/algo/vmacd/VMACD.h"
#include "tests/test_utils/TestUtils.h"

// Shared VMACD params used across all tests
static constexpr int FAST_PERIOD   = 3;
static constexpr int SLOW_PERIOD   = 6;
static constexpr int SIGNAL_PERIOD = 3;

TEST(test_Strategy_VMACD, HoldsUntilBothPriceAndImbalanceInitialised)
{
    VMACD vmacd(VMACD::Params{.fast = FAST_PERIOD, .slow = SLOW_PERIOD, .signal = SIGNAL_PERIOD});

    static constexpr double SEED_PRICE   = 100.0;
    static constexpr int    TRADE_ITERS  = 100;
    static constexpr double PRICE_STEP   = 0.1;
    static constexpr int    QUOTE_ITERS  = 10;
    static constexpr double QUOTE_MID    = 100.0;
    static constexpr double QUOTE_SPREAD = 10.0;
    static constexpr double QUOTE_SIZE   = 1.0;
    static constexpr double QUOTE_IMB    = 0.7;

    // First call seeds prev_valid (Algo behavior) => no processing yet
    EXPECT_EQ(vmacd.generate_trading_signal(make_trade("BTC", SEED_PRICE)), Signal::HOLD);

    // Feed only trades for a while: price updates, but imbalance never initialises => HOLD forever
    bool sawNonHold = false;
    for (int i = 0; i < TRADE_ITERS; ++i)
    {
        Signal sig = vmacd.generate_trading_signal(make_trade("BTC", SEED_PRICE + PRICE_STEP * i));
        if (sig != Signal::HOLD) sawNonHold = true;
    }
    EXPECT_FALSE(sawNonHold);

    // Now start feeding quotes (imbalance) too; still may be in warmup, but should not crash.
    for (int i = 0; i < QUOTE_ITERS; ++i)
    {
        Signal sig = vmacd.generate_trading_signal(
            make_quote_with_imbalance("BTC", ImbalanceQuoteParams{QUOTE_MID, QUOTE_SPREAD, QUOTE_SIZE, QUOTE_IMB}));
        EXPECT_TRUE(sig == Signal::BUY || sig == Signal::SELL || sig == Signal::HOLD);
    }
}

TEST(test_Strategy_VMACD, ProducesBuyAndSellOnRegimeSwings)
{
    VMACD vmacd(VMACD::Params{.fast = FAST_PERIOD, .slow = SLOW_PERIOD, .signal = SIGNAL_PERIOD});

    static constexpr double SEED_PRICE      = 100.0;
    static constexpr double QUOTE_SPREAD    = 10.0;
    static constexpr double QUOTE_SIZE      = 1.0;
    static constexpr double BEARISH_IMB     = -0.9;
    static constexpr double BULLISH_IMB     = +0.9;
    static constexpr int    BEARISH_ITERS   = 80;
    static constexpr int    BULLISH_ITERS   = 120;
    static constexpr int    BEARISH2_ITERS  = 120;
    static constexpr double BEAR_PRICE_STEP = 0.4;
    static constexpr double BULL_START      = 68.0;
    static constexpr double BULL_PRICE_STEP = 0.6;
    static constexpr double BEAR2_START     = 140.0;
    static constexpr double BEAR2_STEP      = 0.7;

    // Seed
    (void)vmacd.generate_trading_signal(make_trade("BTC", SEED_PRICE));

    bool sawBuy  = false;
    bool sawSell = false;

    // Part A: bearish imbalance + downtrend
    for (int i = 0; i < BEARISH_ITERS; ++i)
    {
        (void)vmacd.generate_trading_signal(
            make_quote_with_imbalance("BTC", ImbalanceQuoteParams{SEED_PRICE, QUOTE_SPREAD, QUOTE_SIZE, BEARISH_IMB}));
        Signal sig = vmacd.generate_trading_signal(make_trade("BTC", SEED_PRICE - BEAR_PRICE_STEP * i));
        sawBuy  |= (sig == Signal::BUY);
        sawSell |= (sig == Signal::SELL);
    }

    // Part B: bullish imbalance + uptrend
    for (int i = 0; i < BULLISH_ITERS; ++i)
    {
        (void)vmacd.generate_trading_signal(
            make_quote_with_imbalance("BTC", ImbalanceQuoteParams{SEED_PRICE, QUOTE_SPREAD, QUOTE_SIZE, BULLISH_IMB}));
        Signal sig = vmacd.generate_trading_signal(make_trade("BTC", BULL_START + BULL_PRICE_STEP * i));
        sawBuy  |= (sig == Signal::BUY);
        sawSell |= (sig == Signal::SELL);
    }

    // Part C: bearish imbalance + downtrend again (helps force a SELL if not seen yet)
    for (int i = 0; i < BEARISH2_ITERS; ++i)
    {
        (void)vmacd.generate_trading_signal(
            make_quote_with_imbalance("BTC", ImbalanceQuoteParams{SEED_PRICE, QUOTE_SPREAD, QUOTE_SIZE, BEARISH_IMB}));
        Signal sig = vmacd.generate_trading_signal(make_trade("BTC", BEAR2_START - BEAR2_STEP * i));
        sawBuy  |= (sig == Signal::BUY);
        sawSell |= (sig == Signal::SELL);
    }

    EXPECT_TRUE(sawBuy);
    EXPECT_TRUE(sawSell);
}

TEST(test_Strategy_VMACD, IgnoresInvalidImbalanceInputsAndDoesNotCrash)
{
    VMACD vmacd(VMACD::Params{.fast = FAST_PERIOD, .slow = SLOW_PERIOD, .signal = SIGNAL_PERIOD});

    static constexpr double SEED_PRICE = 100.0;
    static constexpr double VALID_BID  = 99.0;
    static constexpr double VALID_ASK  = 101.0;
    static constexpr double VALID_SIZE = 1.0;
    static constexpr double ZERO_SIZE  = 0.0;
    static constexpr double INV_BID    = 105.0;
    static constexpr double INV_ASK    = 100.0;

    // Seed prev_valid
    (void)vmacd.generate_trading_signal(make_trade("BTC", SEED_PRICE));

    // Valid quote/trade to get things started
    EXPECT_EQ(vmacd.generate_trading_signal(
        make_quote("BTC", QuoteParams{VALID_BID, VALID_ASK, VALID_SIZE, VALID_SIZE})), Signal::HOLD);
    EXPECT_EQ(vmacd.generate_trading_signal(make_trade("BTC", SEED_PRICE)), Signal::HOLD);

    // Invalid quote: missing sizes
    MarketQuote missing_sizes;
    missing_sizes.symbol   = "BTC";
    missing_sizes.bidPrice = VALID_BID;
    missing_sizes.askPrice = VALID_ASK;

    Signal sig1 = vmacd.generate_trading_signal(missing_sizes);
    EXPECT_TRUE(sig1 == Signal::BUY || sig1 == Signal::SELL || sig1 == Signal::HOLD);

    // Invalid quote: denom=0
    Signal sig2 = vmacd.generate_trading_signal(
        make_quote("BTC", QuoteParams{VALID_BID, VALID_ASK, ZERO_SIZE, ZERO_SIZE}));
    EXPECT_TRUE(sig2 == Signal::BUY || sig2 == Signal::SELL || sig2 == Signal::HOLD);

    // Invalid quote: crossed market (bid > ask) should be rejected by Algo::is_genuine_transition()
    Signal sig3 = vmacd.generate_trading_signal(
        make_quote("BTC", QuoteParams{INV_BID, INV_ASK, VALID_SIZE, VALID_SIZE}));
    EXPECT_EQ(sig3, Signal::HOLD);
}
