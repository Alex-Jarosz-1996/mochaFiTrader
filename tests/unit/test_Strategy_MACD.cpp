#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <vector>

#include "src/algo/macd/MACD.h"
#include "src/algo/Signal.h"
#include "src/marketquote/MarketQuote.h"

#include "tests/test_utils/TestUtils.h"

// Shared MACD params used across all tests (trigger_window = 3 * slow = 18)
static constexpr int FAST_PERIOD   = 3;
static constexpr int SLOW_PERIOD   = 6;
static constexpr int SIGNAL_PERIOD = 3;

TEST(test_Strategy_MACD, HoldsDuringWarmupOnValidTicks)
{
    MACD macd(MACD::Params{.fast = FAST_PERIOD, .slow = SLOW_PERIOD, .signal = SIGNAL_PERIOD});

    static constexpr double SEED_PRICE  = 100.0;
    static constexpr int    WARMUP_ITER = 17;

    EXPECT_EQ(macd.generate_trading_signal(make_trade("BTC", SEED_PRICE)), Signal::HOLD);

    for (int i = 0; i < WARMUP_ITER; ++i)
    {
        Signal sig = macd.generate_trading_signal(make_trade("BTC", SEED_PRICE));
        EXPECT_EQ(sig, Signal::HOLD);
    }
}

TEST(test_Strategy_MACD, InvalidTicksDoNotAdvanceWarmupOrCauseSignals)
{
    MACD macd(MACD::Params{.fast = FAST_PERIOD, .slow = SLOW_PERIOD, .signal = SIGNAL_PERIOD});

    static constexpr double SEED_PRICE    = 100.0;
    static constexpr int    INVALID_ITERS = 50;
    static constexpr int    VALID_ITERS   = 10;

    EXPECT_EQ(macd.generate_trading_signal(make_trade("BTC", SEED_PRICE)), Signal::HOLD);

    for (int i = 0; i < INVALID_ITERS; ++i)
    {
        Signal sig = macd.generate_trading_signal(make_unknown("BTC"));
        EXPECT_EQ(sig, Signal::HOLD);
    }

    bool sawNonHold = false;
    for (int i = 0; i < VALID_ITERS; ++i)
    {
        Signal sig = macd.generate_trading_signal(make_trade("BTC", SEED_PRICE));
        if (sig != Signal::HOLD) sawNonHold = true;
    }
    EXPECT_FALSE(sawNonHold);
}

TEST(test_Strategy_MACD, EmitsBuyAndSellOnConstructedSeries)
{
    MACD macd(MACD::Params{.fast = FAST_PERIOD, .slow = SLOW_PERIOD, .signal = SIGNAL_PERIOD});

    static constexpr double SEED_PRICE       = 100.0;
    static constexpr int    FLAT_ITERS       = 10;
    static constexpr int    DOWN_ITERS       = 10;
    static constexpr double DOWN_START       = 125.0;
    static constexpr double DOWN_STEP        = 2.0;
    static constexpr int    UP_ITERS         = 25;
    static constexpr int    STRONG_DOWN_ITER = 25;

    std::vector<Signal> signals;
    signals.reserve(FLAT_ITERS + DOWN_ITERS + UP_ITERS + STRONG_DOWN_ITER + FLAT_ITERS);

    for (int i = 0; i < FLAT_ITERS; ++i)
        signals.push_back(macd.generate_trading_signal(make_trade("BTC", SEED_PRICE)));

    for (int i = 0; i < DOWN_ITERS; ++i)
        signals.push_back(macd.generate_trading_signal(make_trade("BTC", DOWN_START - DOWN_STEP * i)));

    for (int i = 0; i < UP_ITERS; ++i)
        signals.push_back(macd.generate_trading_signal(make_trade("BTC", SEED_PRICE + i)));

    for (int i = 0; i < STRONG_DOWN_ITER; ++i)
        signals.push_back(macd.generate_trading_signal(make_trade("BTC", DOWN_START - DOWN_STEP * i)));

    for (int i = 0; i < FLAT_ITERS; ++i)
        signals.push_back(macd.generate_trading_signal(make_trade("BTC", SEED_PRICE + i)));

    bool sawBuy  = false;
    bool sawSell = false;
    for (Signal sig : signals)
    {
        if (sig == Signal::BUY)  sawBuy  = true;
        if (sig == Signal::SELL) sawSell = true;
    }

    EXPECT_TRUE(sawBuy);
    EXPECT_TRUE(sawSell);
}

TEST(test_Strategy_MACD, WorksWithMixedTradeAndQuotePackets)
{
    MACD macd(MACD::Params{.fast = FAST_PERIOD, .slow = SLOW_PERIOD, .signal = SIGNAL_PERIOD});

    static constexpr double SEED_PRICE   = 100.0;
    static constexpr double PRICE_101    = 101.0;
    static constexpr double PRICE_102    = 102.0;
    static constexpr double PRICE_103    = 103.0;
    static constexpr double TYPICAL_SIZE = 2.0;
    static constexpr double BID_99_5     = 99.5;
    static constexpr double ASK_100_5    = 100.5;
    static constexpr double BID_101_5    = 101.5;
    static constexpr double ASK_102_5    = 102.5;
    static constexpr double BID_102_0    = 102.0;
    static constexpr double ASK_103_0    = 103.0;

    std::vector<MarketQuote> stream = {
        make_trade("BTC", SEED_PRICE),
        make_quote("BTC", QuoteParams{BID_99_5,  ASK_100_5, TYPICAL_SIZE, TYPICAL_SIZE}),
        make_trade("BTC", PRICE_101),
        make_trade("BTC", PRICE_102),
        make_quote("BTC", QuoteParams{BID_101_5, ASK_102_5, TYPICAL_SIZE, TYPICAL_SIZE}),
        make_quote("BTC", QuoteParams{BID_102_0, ASK_103_0, TYPICAL_SIZE, TYPICAL_SIZE}),
        make_trade("BTC", PRICE_103),
    };

    for (const MarketQuote& quote : stream)
    {
        Signal sig = macd.generate_trading_signal(quote);
        EXPECT_TRUE(sig == Signal::BUY || sig == Signal::SELL || sig == Signal::HOLD);
    }
}
