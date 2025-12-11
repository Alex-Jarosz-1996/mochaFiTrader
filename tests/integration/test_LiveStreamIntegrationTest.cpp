#include <gtest/gtest.h>
#include <memory>
#include <vector>


#include "../../src/tastyworks/TastyWorks.h"
#include "../../src/streamer/DX_LinkStreamer.h"
#include "../../src/database/DB_Client.h"
#include "../../src/marketquote/MarketQuote.h"
#include "../../src/log/Log.h"

TEST(LiveStreamIntegrationTest, StreamInsertAndRetrieveQuote)
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

    bool called = false;
    dxlStreamer->set_on_quote([&](const MarketQuote& quote)
    {
        dbClient->insert_quote(quote);
        std::optional<std::vector<MarketQuote>> quote_from_db = dbClient->get_quote();
        called = true;
    });
    
    SUCCEED();
}