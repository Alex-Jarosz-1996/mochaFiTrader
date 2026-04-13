#include <gtest/gtest.h>
#include "../../src/algo/algo/Algo.h"
#include "../test_utils/TestUtils.h"

namespace {
class MockAlgo : public Algo {
public:
    void process_quote() override {} // No-op for testing transitions
};
}

TEST(test_Algo, RejectSignificantPriceJump)
{
    MockAlgo algo;
    MarketQuote initial = make_trade("BTC/USD", 100.0);
    MarketQuote jump = make_trade("BTC/USD", 116.0); // 16% jump

    algo.generate_trading_signal(initial);
    Signal s = algo.generate_trading_signal(jump);

    EXPECT_EQ(algo.get_valid_count(), 0); // Should not have incremented
    EXPECT_EQ(s, Signal::HOLD); // Should be rejected/ignored
}

TEST(test_Algo, AcceptNormalPriceMove)
{
    MockAlgo algo;
    MarketQuote initial = make_trade("BTC/USD", 100.0);
    MarketQuote move = make_trade("BTC/USD", 105.0); // 5% move

    algo.generate_trading_signal(initial);
    Signal s = algo.generate_trading_signal(move);

    EXPECT_EQ(algo.get_valid_count(), 1);
    // If genuine, it processes. Note: MockAlgo does nothing, so it stays HOLD, 
    // but we are testing that the internal state promotes prev_valid.
    EXPECT_EQ(s, Signal::HOLD); 
}

TEST(test_Algo, AcceptsMixedTradeToQuoteTransition)
{
    MockAlgo algo;
    MarketQuote initial = make_trade("BTC/USD", 100.0);
    MarketQuote next_quote = make_quote("BTC/USD", 100.0, 101.0, 1.0, 1.0);

    algo.generate_trading_signal(initial);
    algo.generate_trading_signal(next_quote);

    EXPECT_EQ(algo.get_valid_count(), 1);
}

TEST(test_Algo, RejectInvalidBidAskSpread)
{
    MockAlgo algo;
    // Bid higher than Ask
    MarketQuote invalid = make_quote("BTC/USD", 105.0, 100.0, 1.0, 1.0);
    
    EXPECT_EQ(algo.generate_trading_signal(invalid), Signal::HOLD);
}

TEST(test_Algo, RejectMassiveSpread)
{
    MockAlgo algo;
    // Spread is 30% of bid (limit is 20%)
    MarketQuote wide = make_quote("BTC/USD", 100.0, 130.0, 1.0, 1.0);
    
    EXPECT_EQ(algo.generate_trading_signal(wide), Signal::HOLD);
}

TEST(test_Algo, RecoveryAfterInvalidTransition)
{
    MockAlgo algo;
    MarketQuote first = make_trade("BTC/USD", 100.0);
    MarketQuote bad_jump = make_trade("BTC/USD", 130.0); // Rejected (30% jump)
    MarketQuote second_good = make_trade("BTC/USD", 102.0); // Compared against 'first'

    algo.generate_trading_signal(first);
    algo.generate_trading_signal(bad_jump);
    
    // This should be accepted because 102.0 is a 2% move from 100.0
    algo.generate_trading_signal(second_good);
    EXPECT_EQ(algo.get_valid_count(), 2);
}

TEST(test_Algo, RejectNegativePriceTrade)
{
    MockAlgo algo;
    MarketQuote negative = make_trade("BTC/USD", -1.0);
    EXPECT_EQ(algo.generate_trading_signal(negative), Signal::HOLD);
}

TEST(test_Algo, RejectZeroPriceTrade)
{
    MockAlgo algo;
    MarketQuote zero = make_trade("BTC/USD", 0.0);
    EXPECT_EQ(algo.generate_trading_signal(zero), Signal::HOLD);
}