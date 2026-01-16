#include <gtest/gtest.h>
#include <memory>
#include <vector>


#include "../../src/tastyworks/TastyWorks.h"
#include "../../src/streamer/DX_LinkStreamer.h"
#include "../../src/marketquote/MarketQuote.h"
#include "../../src/log/Log.h"
#include "../../src/algo/macd/MACD.h"

TEST(LiveStreamIntegrationTest, StreamInsertAndRetrieveQuote)
{
    Log::init();
    
    // tastyworks
    std::unique_ptr<TastyWorksClient> twClient = std::make_unique<TastyWorksClient>();
    
    // streamer
    std::unique_ptr<DX_LinkStreamer> dxlStreamer = std::make_unique<DX_LinkStreamer>(
        *twClient
    );

    // strategy
    std::unique_ptr<MACD> strategy = std::make_unique<MACD>();

    bool called = false;
    dxlStreamer->set_on_quote([&](const MarketQuote& quote)
    {
        Signal signal = strategy->generate_trading_signal(quote);
        called = true;
    });
    
    SUCCEED();
}