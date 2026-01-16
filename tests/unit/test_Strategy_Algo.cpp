#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <vector>

#include "../../src/marketquote/MarketQuote.h"

#include "../test_utils/TestUtils.h"

// TickType::Unknown
TEST(test_Strategy_Algo, RejectsUnknown)
{
    AlgoHarness h;
    MarketQuote prev = make_unknown("BTC");
    MarketQuote curr = make_unknown("BTC");

    EXPECT_FALSE(h.is_genuine_transition(prev, curr));
}

// TickType::Trade
TEST(test_Strategy_Algo, RejectsTradePrevValueIsZero)
{
    AlgoHarness h;
    MarketQuote prev = make_trade("BTC", 0.0); // prev value is zero (0.0)
    MarketQuote curr = make_trade("BTC", 120.1);

    EXPECT_FALSE(h.is_genuine_transition(prev, curr));
}

TEST(test_Strategy_Algo, RejectsTradeCurrValueIsZero)
{
    AlgoHarness h;
    MarketQuote prev = make_trade("BTC", 120.0);
    MarketQuote curr = make_trade("BTC", 0.0); // curr value is zero (0.0)

    EXPECT_FALSE(h.is_genuine_transition(prev, curr));
}

TEST(test_Strategy_Algo, AcceptsTradeCurrGreatherThanPrevPriceDeltaLessThanThreshold)
{
    AlgoHarness h;
    MarketQuote prev = make_trade("BTC", 100.0);
    MarketQuote curr = make_trade("BTC", 114.9); // 14.9% price jump

    EXPECT_TRUE(h.is_genuine_transition(prev, curr));
}

TEST(test_Strategy_Algo, AcceptsTradeCurrGreatherThanPrevPriceDeltaEqualsThreshold)
{
    AlgoHarness h;
    MarketQuote prev = make_trade("BTC", 100.0);
    MarketQuote curr = make_trade("BTC", 115.0); // 15% price jump

    EXPECT_TRUE(h.is_genuine_transition(prev, curr));
}

TEST(test_Strategy_Algo, RejectsTradeCurrGreatherThanPrevPriceDeltaGreaterThanThreshold)
{
    AlgoHarness h;
    MarketQuote prev = make_trade("BTC", 100.0);
    MarketQuote curr = make_trade("BTC", 115.1); // 15.1% price jump

    EXPECT_FALSE(h.is_genuine_transition(prev, curr));
}

TEST(test_Strategy_Algo, AcceptsTradeCurrLessThanPrevPriceDeltaLessThanThreshold)
{
    AlgoHarness h;
    MarketQuote prev = make_trade("BTC", 117); // < 15% price jump
    MarketQuote curr = make_trade("BTC", 100.0);

    EXPECT_TRUE(h.is_genuine_transition(prev, curr));
}

TEST(test_Strategy_Algo, AcceptsTradeCurrLessThanPrevPriceDeltaGreaterThanThreshold)
{
    AlgoHarness h;
    MarketQuote prev = make_trade("BTC", 118); // > 15% price jump
    MarketQuote curr = make_trade("BTC", 100.0);

    EXPECT_FALSE(h.is_genuine_transition(prev, curr));
}

// TickType::Quote
TEST(test_Strategy_Algo, RejectsCrossedMarketBidGreaterThanAsk)
{
    AlgoHarness h;
    MarketQuote prev = make_quote("BTC", 99.0, 101.0, 1.0, 1.0);
    MarketQuote curr = make_quote("BTC", 105.0, 100.0, 1.0, 1.0); // bid > ask

    EXPECT_FALSE(h.is_genuine_transition(prev, curr));
}

TEST(test_Strategy_Algo, RejectsTradeNoBidValue)
{
    AlgoHarness h;
    MarketQuote prev = make_quote("BTC", 0.0, 100.0, 1.0, 1.0); // no bid value
    MarketQuote curr = make_quote("BTC", 105.0, 100.0, 1.0, 1.0);

    EXPECT_FALSE(h.is_genuine_transition(prev, curr));
}

TEST(test_Strategy_Algo, RejectsTradeNoAskValue)
{
    AlgoHarness h;
    MarketQuote prev = make_quote("BTC", 90.0, 100.0, 1.0, 1.0); // no bid value
    MarketQuote curr = make_quote("BTC", 0.0, 100.0, 1.0, 1.0);

    EXPECT_FALSE(h.is_genuine_transition(prev, curr));
}

TEST(test_Strategy_Algo, AcceptsTradeBidAskSpreadLessThanThreshold)
{
    AlgoHarness h;
    MarketQuote prev = make_quote("BTC", 90.0, 100.0, 1.0, 1.0);
    MarketQuote curr = make_quote("BTC", 100.0, 119.0, 1.0, 1.0); // ((ask - bid) / bid) < threshold

    EXPECT_TRUE(h.is_genuine_transition(prev, curr));
}

TEST(test_Strategy_Algo, RejectsTradeBidAskSpreadGreaterThanThreshold)
{
    AlgoHarness h;
    MarketQuote prev = make_quote("BTC", 90.0, 91.0, 1.0, 1.0);
    MarketQuote curr = make_quote("BTC", 100.0, 121.0, 1.0, 1.0); // ((ask - bid) / bid) > threshold

    EXPECT_FALSE(h.is_genuine_transition(prev, curr));
}

// mix Quote and Trade
TEST(test_Strategy_Algo, AcceptsMixedTradeToQuoteByDefault)
{
    AlgoHarness h;
    MarketQuote prev = make_trade("BTC", 100.0);
    MarketQuote curr = make_quote("BTC", 99.0, 101.0, 1.0, 1.0);

    EXPECT_TRUE(h.is_genuine_transition(prev, curr));
}
