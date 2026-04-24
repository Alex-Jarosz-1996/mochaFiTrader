#include <gtest/gtest.h>

#include "src/algo/AlgoFactory.h"
#include "src/algo/StrategyAggregator.h"
#include "src/config/Config.h"
#include "src/algo/Signal.h"
#include "src/log/Log.h"
#include "tests/test_utils/TestUtils.h"

TEST(AlgoFactoryIntegration, StrategiesFromConfig_AreNonNull)
{
    Log::init("src/log/");
    const auto names = Config::get_config_array("STRATEGIES");
    const auto strategies = AlgoFactory::make_strategies(names);
    ASSERT_FALSE(strategies.empty());
    for (const auto& s : strategies)
        EXPECT_NE(s, nullptr);
}

TEST(AlgoFactoryIntegration, StrategiesFromConfig_NamesAreKnown)
{
    Log::init("src/log/");
    const auto names = Config::get_config_array("STRATEGIES");
    EXPECT_NO_THROW(AlgoFactory::make_strategies(names));
}

TEST(AlgoFactoryIntegration, StrategyAggregatorFromConfig_ProducesHoldDuringWarmup)
{
    Log::init("src/log/");
    const auto names = Config::get_config_array("STRATEGIES");
    StrategyAggregator agg(AlgoFactory::make_strategies(names));

    MarketQuote quote = make_trade("BTC/USD", 50000.0);
    EXPECT_EQ(agg.generate_trading_signal(quote), Signal::HOLD);
}
