#include <gtest/gtest.h>

#include "src/algo/AlgoFactory.h"
#include "src/config/Config.h"
#include "src/algo/Signal.h"
#include "src/log/Log.h"
#include "tests/test_utils/TestUtils.h"

TEST(AlgoFactoryIntegration, StrategyFromConfig_IsNonNull)
{
    Log::init("src/log/");
    const std::string strategy_name = Config::get_config_value("STRATEGY");
    auto strategy = AlgoFactory::make_strategy(strategy_name);
    ASSERT_NE(strategy, nullptr);
}

TEST(AlgoFactoryIntegration, StrategyFromConfig_ProducesHoldDuringWarmup)
{
    Log::init("src/log/");
    const std::string strategy_name = Config::get_config_value("STRATEGY");
    auto strategy = AlgoFactory::make_strategy(strategy_name);

    MarketQuote quote = make_trade("BTC/USD", 50000.0);
    EXPECT_EQ(strategy->generate_trading_signal(quote), Signal::HOLD);
}

TEST(AlgoFactoryIntegration, StrategyFromConfig_NameIsKnown)
{
    Log::init("src/log/");
    const std::string strategy_name = Config::get_config_value("STRATEGY");
    EXPECT_NO_THROW(AlgoFactory::make_strategy(strategy_name));
}
