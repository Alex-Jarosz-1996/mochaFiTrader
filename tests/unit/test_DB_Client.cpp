#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "../../src/database/DB_Client.h"
#include "../../src/log/Log.h"
#include "../../src/marketquote/MarketQuote.h"

class DBClientTestFixture : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Log::init("src/log/");
    }
};

TEST_F(DBClientTestFixture, CanConstructDBClient)
{
    EXPECT_NO_THROW({
        std::unique_ptr<DB_Client> dbClient = std::make_unique<DB_Client>();
    });
}

TEST_F(DBClientTestFixture, CanInsertQuoteDBClient)
{
    std::unique_ptr<DB_Client> dbClient = std::make_unique<DB_Client>();

    MarketQuote q;
    q.symbol = "BTC/USD";
    q.price = 100.5;
    q.bidPrice = 100.0;
    q.askPrice = 101.0;
    q.dayVolume = 123456.0;
    q.size = 12321.0;
    q.bidSize = 12000.0;
    q.askSize = 5000.0;

    EXPECT_NO_THROW({
        dbClient->insert_quote(q);
    });
}

TEST_F(DBClientTestFixture, CanInsertAndReadQuoteDBClient)
{
    std::unique_ptr<DB_Client> dbClient = std::make_unique<DB_Client>();

    MarketQuote q;
    q.symbol = "BTC/USD";
    q.price = 100.5;
    q.bidPrice = 100.0;
    q.askPrice = 101.0;
    q.dayVolume = 123456.0;
    q.size = 12321.0;
    q.bidSize = 12000.0;
    q.askSize = 5000.0;

    dbClient->insert_quote(q);

    std::optional<std::vector<MarketQuote>> qts = dbClient->get_quote();

    ASSERT_TRUE(qts.has_value());
    ASSERT_GT(qts->size(), 0);

    const MarketQuote& qt = qts->at(0);

    EXPECT_EQ(qt.symbol, "BTC/USD");
    EXPECT_EQ(qt.price.value(), 100.5);
    EXPECT_EQ(qt.bidPrice.value(), 100.0);
    EXPECT_EQ(qt.askPrice.value(), 101.0);
    EXPECT_EQ(qt.dayVolume.value(), 123456.0);
    EXPECT_EQ(qt.size.value(), 12321.0);
    EXPECT_EQ(qt.bidSize.value(), 12000.0);
    EXPECT_EQ(qt.askSize.value(), 5000.0);
}
