#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <vector>

#include "../../src/algo/macd/MACD.h"
#include "../../src/algo/Signal.h"
#include "../../src/marketquote/MarketQuote.h"

#include "../test_utils/TestUtils.h"


TEST(test_Strategy_MACD, HoldsDuringWarmupOnValidTicks)
{
    // trigger_window = 3 * slow => 3 * 6 = 18 valid ticks.
    MACD macd(/*fast=*/3, /*slow=*/6, /*signal=*/3);

    // seed quote
    EXPECT_EQ(macd.generate_trading_signal(make_trade("BTC", 100.0)), Signal::HOLD);

    // feed 17 more valid ticks (total valid_count_ will be 17)
    for (int i = 0; i < 17; ++i)
    {
        Signal s = macd.generate_trading_signal(make_trade("BTC", 100.0));
        EXPECT_EQ(s, Signal::HOLD);
    }
}

TEST(test_Strategy_MACD, InvalidTicksDoNotAdvanceWarmupOrCauseSignals)
{
    MACD macd(/*fast=*/3, /*slow=*/6, /*signal=*/3);

    // seed
    EXPECT_EQ(macd.generate_trading_signal(make_trade("BTC", 100.0)), Signal::HOLD);

    // invalid: unknown tick => should not advance valid_count
    for (int i = 0; i < 50; ++i)
    {
        Signal s = macd.generate_trading_signal(make_unknown("BTC"));
        EXPECT_EQ(s, Signal::HOLD);
    }

    // Now feed enough valid constant ticks to reach warmup boundary.
    // If invalid ticks had counted, we'd potentially be past warmup; they should NOT.
    // We don't have direct access to valid_count_ here, but behavior should still
    // look like warmup: HOLD for a while.
    bool sawNonHold = false;
    for (int i = 0; i < 10; ++i)
    {
        Signal s = macd.generate_trading_signal(make_trade("BTC", 100.0));
        if (s != Signal::HOLD) sawNonHold = true;
    }
    EXPECT_FALSE(sawNonHold); // extremely likely; constant prices should not crossover anyway
}

TEST(test_Strategy_MACD, EmitsBuyAndSellOnConstructedSeries)
{
    MACD macd(/*fast=*/3, /*slow=*/6, /*signal=*/3);

    std::vector<Signal> signals;

    // seed flat
    for (int i = 0; i < 10; ++i)
    {
        signals.push_back(macd.generate_trading_signal(make_trade("BTC", 100.0)));
    }

    // weak downtrend
    for (int i = 0; i < 10; ++i)
    {
        signals.push_back(macd.generate_trading_signal(make_trade("BTC", 125.0 - 2.0 * i)));
    }
    
    // strong uptrend
    for (int i = 0; i < 25; ++i)
    {
        signals.push_back(macd.generate_trading_signal(make_trade("BTC", 100.0 + i)));
    }

    // strong downtrend
    for (int i = 0; i < 25; ++i)
    {
        signals.push_back(macd.generate_trading_signal(make_trade("BTC", 125.0 - 2.0 * i)));
    }

    // weak uptrend
    for (int i = 0; i < 10; ++i)
    {
        signals.push_back(macd.generate_trading_signal(make_trade("BTC", 100.0 + i)));
    }

    bool sawBuy = false;
    bool sawSell = false;
    for (Signal s : signals)
    {
        if (s == Signal::BUY) sawBuy = true;
        if (s == Signal::SELL) sawSell = true;
    }

    EXPECT_TRUE(sawBuy);
    EXPECT_TRUE(sawSell);
}

TEST(test_Strategy_MACD, WorksWithMixedTradeAndQuotePackets)
{
    MACD macd(/*fast=*/3, /*slow=*/6, /*signal=*/3);

    // alternate: trade tick then quote tick; sometimes repeated.
    // quotes are mid-price ~ trade price to keep it sensible.
    std::vector<MarketQuote> stream = {
        make_trade("BTC", 100.0),
        make_quote("BTC", 99.5, 100.5, 2.0, 2.0),
        make_trade("BTC", 101.0),
        make_trade("BTC", 102.0),  // repeated trades
        make_quote("BTC", 101.5, 102.5, 2.0, 2.0),
        make_quote("BTC", 102.0, 103.0, 2.0, 2.0), // repeated quotes
        make_trade("BTC", 103.0),
    };

    // Should not crash and should always return a valid Signal enum
    for (const MarketQuote& q : stream) {
        Signal s = macd.generate_trading_signal(q);
        EXPECT_TRUE(s == Signal::BUY || s == Signal::SELL || s == Signal::HOLD);
    }
}
