#include <gtest/gtest.h>
#include "src/algo/algo/Algo.h"
#include "tests/test_utils/TestUtils.h"

namespace {
class MockAlgo : public Algo {
public:
    void process_quote() override {} // No-op for testing transitions
};
}

TEST(test_Algo, RejectSignificantPriceJump)
{
    static constexpr double BASE_PRICE = 100.0;
    static constexpr double JUMP_PRICE = 116.0; // 16% jump

    MockAlgo algo;
    MarketQuote initial = make_trade("BTC/USD", BASE_PRICE);
    MarketQuote jump    = make_trade("BTC/USD", JUMP_PRICE);

    algo.generate_trading_signal(initial);
    Signal sig = algo.generate_trading_signal(jump);

    EXPECT_EQ(algo.get_valid_count(), 0);
    EXPECT_EQ(sig, Signal::HOLD);
}

TEST(test_Algo, AcceptNormalPriceMove)
{
    static constexpr double BASE_PRICE = 100.0;
    static constexpr double MOVE_PRICE = 105.0; // 5% move

    MockAlgo algo;
    MarketQuote initial = make_trade("BTC/USD", BASE_PRICE);
    MarketQuote move    = make_trade("BTC/USD", MOVE_PRICE);

    algo.generate_trading_signal(initial);
    Signal sig = algo.generate_trading_signal(move);

    EXPECT_EQ(algo.get_valid_count(), 1);
    EXPECT_EQ(sig, Signal::HOLD);
}

TEST(test_Algo, AcceptsMixedTradeToQuoteTransition)
{
    static constexpr double BASE_PRICE = 100.0;
    static constexpr double ASK_PRICE  = 101.0;
    static constexpr double VALID_SIZE = 1.0;

    MockAlgo algo;
    MarketQuote initial    = make_trade("BTC/USD", BASE_PRICE);
    MarketQuote next_quote = make_quote("BTC/USD", QuoteParams{BASE_PRICE, ASK_PRICE, VALID_SIZE, VALID_SIZE});

    algo.generate_trading_signal(initial);
    algo.generate_trading_signal(next_quote);

    EXPECT_EQ(algo.get_valid_count(), 1);
}

TEST(test_Algo, RejectInvalidBidAskSpread)
{
    static constexpr double INV_BID    = 105.0;
    static constexpr double INV_ASK    = 100.0;
    static constexpr double VALID_SIZE = 1.0;

    MockAlgo algo;
    MarketQuote invalid = make_quote("BTC/USD", QuoteParams{INV_BID, INV_ASK, VALID_SIZE, VALID_SIZE});

    EXPECT_EQ(algo.generate_trading_signal(invalid), Signal::HOLD);
}

TEST(test_Algo, RejectMassiveSpread)
{
    static constexpr double BASE_BID   = 100.0;
    static constexpr double WIDE_ASK   = 130.0; // Spread is 30% of bid (limit is 20%)
    static constexpr double VALID_SIZE = 1.0;

    MockAlgo algo;
    MarketQuote wide = make_quote("BTC/USD", QuoteParams{BASE_BID, WIDE_ASK, VALID_SIZE, VALID_SIZE});

    EXPECT_EQ(algo.generate_trading_signal(wide), Signal::HOLD);
}

TEST(test_Algo, RecoveryAfterInvalidTransition)
{
    static constexpr double BASE_PRICE  = 100.0;
    static constexpr double BAD_JUMP    = 130.0; // Rejected (30% jump)
    static constexpr double SECOND_GOOD = 102.0; // 2% move from base — accepted

    MockAlgo algo;
    MarketQuote first       = make_trade("BTC/USD", BASE_PRICE);
    MarketQuote bad         = make_trade("BTC/USD", BAD_JUMP);
    MarketQuote second_good = make_trade("BTC/USD", SECOND_GOOD);

    algo.generate_trading_signal(first);
    algo.generate_trading_signal(bad);

    algo.generate_trading_signal(second_good);
    EXPECT_EQ(algo.get_valid_count(), 1);
}

TEST(test_Algo, RejectNegativePriceTrade)
{
    static constexpr double NEG_PRICE = -1.0;

    MockAlgo algo;
    MarketQuote negative = make_trade("BTC/USD", NEG_PRICE);
    EXPECT_EQ(algo.generate_trading_signal(negative), Signal::HOLD);
}

TEST(test_Algo, RejectZeroPriceTrade)
{
    MockAlgo algo;
    MarketQuote zero = make_trade("BTC/USD", 0.0);
    EXPECT_EQ(algo.generate_trading_signal(zero), Signal::HOLD);
}
