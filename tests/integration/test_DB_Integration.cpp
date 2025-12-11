#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "../../src/tastyworks/TastyWorks.h"
#include "../../src/streamer/DX_LinkStreamer.h"
#include "../../src/database/DB_Client.h"
#include "../../src/marketquote/MarketQuote.h"
#include "../../src/log/Log.h"

TEST(DBIntegrationTest, DBInsertAndRetrieveQuote)
{
    Log::init();
    
    // db
    std::unique_ptr<DB_Client> dbClient = std::make_unique<DB_Client>();
    
    // tastyworks
    std::unique_ptr<TastyWorksClient> twClient = std::make_unique<TastyWorksClient>();
    
    // streamer
    std::unique_ptr<DX_LinkStreamer> dxlStreamer = std::make_unique<DX_LinkStreamer>(
        *twClient
    );

    // defining quote
    MarketQuote q;
    q.symbol = "BTC/USD";
    q.price = 100.5;
    q.bidPrice = 100.0;
    q.askPrice = 101.0;
    q.dayVolume = 123456.0;
    q.size = 12321.0;
    q.bidSize = 12000.0;
    q.askSize = 5000.0;

    // inserting data
    dbClient->insert_quote(q);

    // retrieving data
    std::optional<std::vector<MarketQuote>> quote_from_db = dbClient->get_quote();

    SUCCEED();
}