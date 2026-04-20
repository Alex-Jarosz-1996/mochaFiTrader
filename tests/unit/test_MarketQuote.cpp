#include <gtest/gtest.h>
#include "../../src/marketquote/MarketQuote.h"

TEST(test_MarketQuote, DefaultIsNullOrEmptyValue)
{
    MarketQuote mkt;
    EXPECT_TRUE(mkt.symbol.empty());
    EXPECT_FALSE(mkt.price.has_value());
    EXPECT_FALSE(mkt.bidPrice.has_value());
    EXPECT_FALSE(mkt.askPrice.has_value());
    EXPECT_FALSE(mkt.dayVolume.has_value());
    EXPECT_FALSE(mkt.size.has_value());
    EXPECT_FALSE(mkt.bidSize.has_value());
    EXPECT_FALSE(mkt.askSize.has_value());
}

TEST(test_MarketQuote, CanSetAndReadSymbolValue)
{
    MarketQuote mkt;
    mkt.symbol = "BTC/USD";
    ASSERT_FALSE(mkt.symbol.empty());
    EXPECT_EQ(mkt.symbol, "BTC/USD");
}

TEST(test_MarketQuote, CanSetAndReadPriceValue)
{
    static constexpr double TEST_PRICE = 100.5;
    MarketQuote mkt;
    mkt.price = TEST_PRICE;
    ASSERT_TRUE(mkt.price.has_value());
    EXPECT_EQ(mkt.price, TEST_PRICE);
}

TEST(test_MarketQuote, CanSetAndReadBidPriceValue)
{
    static constexpr double TEST_BID = 100.0;
    MarketQuote mkt;
    mkt.bidPrice = TEST_BID;
    ASSERT_TRUE(mkt.bidPrice.has_value());
    EXPECT_EQ(mkt.bidPrice, TEST_BID);
}

TEST(test_MarketQuote, CanSetAndReadAskPriceValue)
{
    static constexpr double TEST_ASK = 101.0;
    MarketQuote mkt;
    mkt.askPrice = TEST_ASK;
    ASSERT_TRUE(mkt.askPrice.has_value());
    EXPECT_EQ(mkt.askPrice, TEST_ASK);
}

TEST(test_MarketQuote, CanSetAndReadDayVolumeValue)
{
    static constexpr double TEST_DAY_VOLUME = 123456.0;
    MarketQuote mkt;
    mkt.dayVolume = TEST_DAY_VOLUME;
    ASSERT_TRUE(mkt.dayVolume.has_value());
    EXPECT_EQ(mkt.dayVolume, TEST_DAY_VOLUME);
}

TEST(test_MarketQuote, CanSetAndReadSizeValue)
{
    static constexpr double TEST_SIZE = 12321.0;
    MarketQuote mkt;
    mkt.size = TEST_SIZE;
    ASSERT_TRUE(mkt.size.has_value());
    EXPECT_EQ(mkt.size, TEST_SIZE);
}

TEST(test_MarketQuote, CanSetAndReadBidSizeValue)
{
    static constexpr double TEST_BID_SIZE = 12000.0;
    MarketQuote mkt;
    mkt.bidSize = TEST_BID_SIZE;
    ASSERT_TRUE(mkt.bidSize.has_value());
    EXPECT_EQ(mkt.bidSize, TEST_BID_SIZE);
}

TEST(test_MarketQuote, CanSetAndReadAskSizeValue)
{
    static constexpr double TEST_ASK_SIZE = 5000.0;
    MarketQuote mkt;
    mkt.askSize = TEST_ASK_SIZE;
    ASSERT_TRUE(mkt.askSize.has_value());
    EXPECT_EQ(mkt.askSize, TEST_ASK_SIZE);
}
