#include <gtest/gtest.h>
#include <stdexcept>

#include "src/algo/AlgoFactory.h"
#include "src/algo/vmacd/VMACD.h"
#include "src/algo/macd/MACD.h"
#include "src/algo/rsi/RSI.h"
#include "tests/test_utils/TestUtils.h"

TEST(test_AlgoFactory, MakeStrategy_Vmacd_ReturnsNonNull)
{
    auto strategy = AlgoFactory::make_strategy("vmacd");
    ASSERT_NE(strategy, nullptr);
}

TEST(test_AlgoFactory, MakeStrategy_Macd_ReturnsNonNull)
{
    auto strategy = AlgoFactory::make_strategy("macd");
    ASSERT_NE(strategy, nullptr);
}

TEST(test_AlgoFactory, MakeStrategy_Rsi_ReturnsNonNull)
{
    auto strategy = AlgoFactory::make_strategy("rsi");
    ASSERT_NE(strategy, nullptr);
}

TEST(test_AlgoFactory, MakeStrategy_UnknownName_ThrowsInvalidArgument)
{
    EXPECT_THROW(AlgoFactory::make_strategy("unknown_strategy"), std::invalid_argument);
}

TEST(test_AlgoFactory, MakeStrategy_EmptyName_ThrowsInvalidArgument)
{
    EXPECT_THROW(AlgoFactory::make_strategy(""), std::invalid_argument);
}

TEST(test_AlgoFactory, MakeStrategy_Vmacd_HoldsOnFirstQuote)
{
    auto strategy = AlgoFactory::make_strategy("vmacd");
    MarketQuote quote = make_trade("BTC/USD", 50000.0);
    EXPECT_EQ(strategy->generate_trading_signal(quote), Signal::HOLD);
}

TEST(test_AlgoFactory, MakeStrategy_Macd_HoldsOnFirstQuote)
{
    auto strategy = AlgoFactory::make_strategy("macd");
    MarketQuote quote = make_trade("BTC/USD", 50000.0);
    EXPECT_EQ(strategy->generate_trading_signal(quote), Signal::HOLD);
}

TEST(test_AlgoFactory, MakeStrategy_Rsi_HoldsOnFirstQuote)
{
    auto strategy = AlgoFactory::make_strategy("rsi");
    MarketQuote quote = make_trade("BTC/USD", 50000.0);
    EXPECT_EQ(strategy->generate_trading_signal(quote), Signal::HOLD);
}

TEST(test_AlgoFactory, MakeStrategies_ReturnsCorrectCount)
{
    auto strategies = AlgoFactory::make_strategies({"vmacd", "macd", "rsi"});
    EXPECT_EQ(strategies.size(), 3U);
}

TEST(test_AlgoFactory, MakeStrategies_AllNonNull)
{
    auto strategies = AlgoFactory::make_strategies({"vmacd", "macd", "rsi"});
    for (const auto& s : strategies)
        EXPECT_NE(s, nullptr);
}

TEST(test_AlgoFactory, MakeStrategies_UnknownName_Throws)
{
    EXPECT_THROW(AlgoFactory::make_strategies({"vmacd", "unknown"}), std::invalid_argument);
}

TEST(test_AlgoFactory, MakeStrategies_EmptyList_ReturnsEmptyVector)
{
    auto strategies = AlgoFactory::make_strategies({});
    EXPECT_TRUE(strategies.empty());
}
