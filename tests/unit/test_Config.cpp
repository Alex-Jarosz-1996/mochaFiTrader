#include <gtest/gtest.h>
#include "../../src/config/Config.h"

TEST(ConfigTest, TastyWorksBaseUrlConfigCheck)
{
    std::string val = Config::get_config_value("TASTYWORKS_BASE_URL");
    EXPECT_FALSE(val.empty());
}

TEST(ConfigTest, TastyWorksAccountNumberConfigCheck)
{
    std::string val = Config::get_config_value("TASTYWORKS_ACCOUNT_NUMBER");
    EXPECT_FALSE(val.empty());
}

TEST(ConfigTest, TastyWorksLoginConfigCheck)
{
    std::string val = Config::get_config_value("TASTYWORKS_LOGIN");
    EXPECT_FALSE(val.empty());
}

TEST(ConfigTest, TastyWorksPasswordConfigCheck)
{
    std::string val = Config::get_config_value("TASTYWORKS_PASSWORD");
    EXPECT_FALSE(val.empty());
}

TEST(ConfigTest, DbCredentialsConfigCheck)
{
    std::string val = Config::get_config_value("DB_CREDENTIALS");
    EXPECT_FALSE(val.empty());
}

TEST(ConfigTest, TradeableAssetsConfigCheck)
{
    std::string val = Config::get_config_value("TRADEABLE_ASSETS");
    EXPECT_FALSE(val.empty());
}

TEST(ConfigTest, InstrumentTypeConfigCheck)
{
    std::string val = Config::get_config_value("INSTRUMENT_TYPE");
    EXPECT_FALSE(val.empty());
}
