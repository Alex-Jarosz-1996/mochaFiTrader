#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>

#include "src/algo/StrategyAggregator.h"
#include "src/algo/Signal.h"
#include "tests/test_utils/TestUtils.h"

namespace {

class FixedSignalAlgo : public Algo
{
public:
    explicit FixedSignalAlgo(Signal fixed) : fixed_(fixed) {}

    Signal generate_trading_signal(const MarketQuote& /*quote*/) override
    {
        return fixed_;
    }

protected:
    void process_quote() override {}

private:
    Signal fixed_;
};

auto make_fixed(Signal s) -> std::unique_ptr<Algo>
{
    return std::make_unique<FixedSignalAlgo>(s);
}

} // namespace

// --- Empty constructor ---

TEST(test_StrategyAggregator, EmptyStrategies_Throws)
{
    std::vector<std::unique_ptr<Algo>> empty;
    EXPECT_THROW(StrategyAggregator(std::move(empty)), std::invalid_argument);
}

// --- Single strategy passthrough ---

TEST(test_StrategyAggregator, SingleStrategy_Buy_ReturnsBuy)
{
    std::vector<std::unique_ptr<Algo>> strategies;
    strategies.push_back(make_fixed(Signal::BUY));
    StrategyAggregator agg(std::move(strategies));

    MarketQuote quote = make_trade("BTC/USD", 50000.0);
    EXPECT_EQ(agg.generate_trading_signal(quote), Signal::BUY);
}

TEST(test_StrategyAggregator, SingleStrategy_Sell_ReturnsSell)
{
    std::vector<std::unique_ptr<Algo>> strategies;
    strategies.push_back(make_fixed(Signal::SELL));
    StrategyAggregator agg(std::move(strategies));

    MarketQuote quote = make_trade("BTC/USD", 50000.0);
    EXPECT_EQ(agg.generate_trading_signal(quote), Signal::SELL);
}

TEST(test_StrategyAggregator, SingleStrategy_Hold_ReturnsHold)
{
    std::vector<std::unique_ptr<Algo>> strategies;
    strategies.push_back(make_fixed(Signal::HOLD));
    StrategyAggregator agg(std::move(strategies));

    MarketQuote quote = make_trade("BTC/USD", 50000.0);
    EXPECT_EQ(agg.generate_trading_signal(quote), Signal::HOLD);
}

// --- Two strategy unanimous combinations (all 9 permutations) ---

TEST(test_StrategyAggregator, TwoStrategies_BuyBuy_ReturnsBuy)
{
    std::vector<std::unique_ptr<Algo>> strategies;
    strategies.push_back(make_fixed(Signal::BUY));
    strategies.push_back(make_fixed(Signal::BUY));
    StrategyAggregator agg(std::move(strategies));

    MarketQuote quote = make_trade("BTC/USD", 50000.0);
    EXPECT_EQ(agg.generate_trading_signal(quote), Signal::BUY);
}

TEST(test_StrategyAggregator, TwoStrategies_SellSell_ReturnsSell)
{
    std::vector<std::unique_ptr<Algo>> strategies;
    strategies.push_back(make_fixed(Signal::SELL));
    strategies.push_back(make_fixed(Signal::SELL));
    StrategyAggregator agg(std::move(strategies));

    MarketQuote quote = make_trade("BTC/USD", 50000.0);
    EXPECT_EQ(agg.generate_trading_signal(quote), Signal::SELL);
}

TEST(test_StrategyAggregator, TwoStrategies_HoldHold_ReturnsHold)
{
    std::vector<std::unique_ptr<Algo>> strategies;
    strategies.push_back(make_fixed(Signal::HOLD));
    strategies.push_back(make_fixed(Signal::HOLD));
    StrategyAggregator agg(std::move(strategies));

    MarketQuote quote = make_trade("BTC/USD", 50000.0);
    EXPECT_EQ(agg.generate_trading_signal(quote), Signal::HOLD);
}

TEST(test_StrategyAggregator, TwoStrategies_BuySell_ReturnsHold)
{
    std::vector<std::unique_ptr<Algo>> strategies;
    strategies.push_back(make_fixed(Signal::BUY));
    strategies.push_back(make_fixed(Signal::SELL));
    StrategyAggregator agg(std::move(strategies));

    MarketQuote quote = make_trade("BTC/USD", 50000.0);
    EXPECT_EQ(agg.generate_trading_signal(quote), Signal::HOLD);
}

TEST(test_StrategyAggregator, TwoStrategies_SellBuy_ReturnsHold)
{
    std::vector<std::unique_ptr<Algo>> strategies;
    strategies.push_back(make_fixed(Signal::SELL));
    strategies.push_back(make_fixed(Signal::BUY));
    StrategyAggregator agg(std::move(strategies));

    MarketQuote quote = make_trade("BTC/USD", 50000.0);
    EXPECT_EQ(agg.generate_trading_signal(quote), Signal::HOLD);
}

TEST(test_StrategyAggregator, TwoStrategies_BuyHold_ReturnsHold)
{
    std::vector<std::unique_ptr<Algo>> strategies;
    strategies.push_back(make_fixed(Signal::BUY));
    strategies.push_back(make_fixed(Signal::HOLD));
    StrategyAggregator agg(std::move(strategies));

    MarketQuote quote = make_trade("BTC/USD", 50000.0);
    EXPECT_EQ(agg.generate_trading_signal(quote), Signal::HOLD);
}

TEST(test_StrategyAggregator, TwoStrategies_HoldBuy_ReturnsHold)
{
    std::vector<std::unique_ptr<Algo>> strategies;
    strategies.push_back(make_fixed(Signal::HOLD));
    strategies.push_back(make_fixed(Signal::BUY));
    StrategyAggregator agg(std::move(strategies));

    MarketQuote quote = make_trade("BTC/USD", 50000.0);
    EXPECT_EQ(agg.generate_trading_signal(quote), Signal::HOLD);
}

TEST(test_StrategyAggregator, TwoStrategies_SellHold_ReturnsHold)
{
    std::vector<std::unique_ptr<Algo>> strategies;
    strategies.push_back(make_fixed(Signal::SELL));
    strategies.push_back(make_fixed(Signal::HOLD));
    StrategyAggregator agg(std::move(strategies));

    MarketQuote quote = make_trade("BTC/USD", 50000.0);
    EXPECT_EQ(agg.generate_trading_signal(quote), Signal::HOLD);
}

TEST(test_StrategyAggregator, TwoStrategies_HoldSell_ReturnsHold)
{
    std::vector<std::unique_ptr<Algo>> strategies;
    strategies.push_back(make_fixed(Signal::HOLD));
    strategies.push_back(make_fixed(Signal::SELL));
    StrategyAggregator agg(std::move(strategies));

    MarketQuote quote = make_trade("BTC/USD", 50000.0);
    EXPECT_EQ(agg.generate_trading_signal(quote), Signal::HOLD);
}

// --- Three strategy unanimous ---

TEST(test_StrategyAggregator, ThreeStrategies_AllBuy_ReturnsBuy)
{
    std::vector<std::unique_ptr<Algo>> strategies;
    strategies.push_back(make_fixed(Signal::BUY));
    strategies.push_back(make_fixed(Signal::BUY));
    strategies.push_back(make_fixed(Signal::BUY));
    StrategyAggregator agg(std::move(strategies));

    MarketQuote quote = make_trade("BTC/USD", 50000.0);
    EXPECT_EQ(agg.generate_trading_signal(quote), Signal::BUY);
}

TEST(test_StrategyAggregator, ThreeStrategies_TwoBuyOneHold_ReturnsHold)
{
    std::vector<std::unique_ptr<Algo>> strategies;
    strategies.push_back(make_fixed(Signal::BUY));
    strategies.push_back(make_fixed(Signal::BUY));
    strategies.push_back(make_fixed(Signal::HOLD));
    StrategyAggregator agg(std::move(strategies));

    MarketQuote quote = make_trade("BTC/USD", 50000.0);
    EXPECT_EQ(agg.generate_trading_signal(quote), Signal::HOLD);
}
