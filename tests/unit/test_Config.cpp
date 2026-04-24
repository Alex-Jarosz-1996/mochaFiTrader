#include <gtest/gtest.h>
#include <stdexcept>
#include "src/config/Config.h"

TEST(test_Config, TastyWorksBaseUrlConfigCheck)
{
    std::string val = Config::get_config_value("TASTYWORKS_BASE_URL");
    EXPECT_FALSE(val.empty());
}

TEST(test_Config, TastyWorksAccountNumberConfigCheck)
{
    std::string val = Config::get_config_value("TASTYWORKS_ACCOUNT_NUMBER");
    EXPECT_FALSE(val.empty());
}

TEST(test_Config, TastyWorksLoginConfigCheck)
{
    std::string val = Config::get_config_value("TASTYWORKS_LOGIN");
    EXPECT_FALSE(val.empty());
}

TEST(test_Config, TastyWorksPasswordConfigCheck)
{
    std::string val = Config::get_config_value("TASTYWORKS_PASSWORD");
    EXPECT_FALSE(val.empty());
}

TEST(test_Config, TradeableAssetsConfigCheck)
{
    std::string val = Config::get_config_value("TRADEABLE_ASSETS");
    EXPECT_FALSE(val.empty());
}

TEST(test_Config, Instrument_typeConfigCheck)
{
    std::string val = Config::get_config_value("INSTRUMENT_TYPE");
    EXPECT_FALSE(val.empty());
}

TEST(test_Config, InstrumentConfigCheck)
{
    std::string val = Config::get_config_value("INSTRUMENT");
    EXPECT_FALSE(val.empty());
}

TEST(test_Config, ThrowsOnMissingKey)
{
    EXPECT_THROW(Config::get_config_value("NON_EXISTENT_KEY_999"), std::invalid_argument);
}

TEST(test_Config, StrategiesArrayPresent)
{
    auto strategies = Config::get_config_array("STRATEGIES");
    EXPECT_FALSE(strategies.empty());
}

TEST(test_Config, GetConfigArray_ThrowsOnMissingKey)
{
    EXPECT_THROW(Config::get_config_array("NON_EXISTENT_ARRAY_999"), std::invalid_argument);
}
