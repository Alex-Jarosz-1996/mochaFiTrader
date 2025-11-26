#include <gtest/gtest.h>
#include "../../src/marketquote/MarketQuote.h"

TEST(MarketQuoteTest, DefaultIsNullOrEmptyValue)
{
    MarketQuote q;
    EXPECT_TRUE(q.symbol.empty());
    EXPECT_FALSE(q.price.has_value());
    EXPECT_FALSE(q.bidPrice.has_value());
    EXPECT_FALSE(q.askPrice.has_value());
    EXPECT_FALSE(q.dayVolume.has_value());
    EXPECT_FALSE(q.size.has_value());
    EXPECT_FALSE(q.bidSize.has_value());
    EXPECT_FALSE(q.askSize.has_value());
}

TEST(MarketQuoteTest, CanSetAndReadSymbolValue)
{
    MarketQuote q;
    q.symbol = "BTC/USD";
    ASSERT_FALSE(q.symbol.empty());
    EXPECT_EQ(q.symbol, "BTC/USD");
}

TEST(MarketQuoteTest, CanSetAndReadPriceValue)
{
    MarketQuote q;
    q.price = 100.5;
    ASSERT_TRUE(q.price.has_value());
    EXPECT_EQ(q.price, 100.5);
}

TEST(MarketQuoteTest, CanSetAndReadBidPriceValue)
{
    MarketQuote q;
    q.bidPrice = 100.0;
    ASSERT_TRUE(q.bidPrice.has_value());
    EXPECT_EQ(q.bidPrice, 100.0);
}

TEST(MarketQuoteTest, CanSetAndReadAskPriceValue)
{
    MarketQuote q;
    q.askPrice = 101.0;
    ASSERT_TRUE(q.askPrice.has_value());
    EXPECT_EQ(q.askPrice, 101.0);
}

TEST(MarketQuoteTest, CanSetAndReadDayVolumeValue)
{
    MarketQuote q;
    q.dayVolume = 123456.0;
    ASSERT_TRUE(q.dayVolume.has_value());
    EXPECT_EQ(q.dayVolume, 123456.0);
}

TEST(MarketQuoteTest, CanSetAndReadSizeValue)
{
    MarketQuote q;
    q.size = 12321.0;
    ASSERT_TRUE(q.size.has_value());
    EXPECT_EQ(q.size, 12321.0);
}

TEST(MarketQuoteTest, CanSetAndReadBidSizeValue)
{
    MarketQuote q;
    q.bidSize = 12000.0;
    ASSERT_TRUE(q.bidSize.has_value());
    EXPECT_EQ(q.bidSize, 12000.0);
}

TEST(MarketQuoteTest, CanSetAndReadAskSizeValue)
{
    MarketQuote q;
    q.askSize = 5000.0;
    ASSERT_TRUE(q.askSize.has_value());
    EXPECT_EQ(q.askSize, 5000.0);
}
