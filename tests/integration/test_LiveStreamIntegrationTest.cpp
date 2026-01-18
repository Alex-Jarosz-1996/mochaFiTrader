#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "../test_utils/MockStreamer.h"
#include "../test_utils/TestUtils.h"
#include "../../src/tastyworks/TastyWorks.h"
#include "../../src/streamer/DX_LinkStreamer.h"
#include "../../src/marketquote/MarketQuote.h"
#include "../../src/log/Log.h"
#include "../../src/algo/macd/MACD.h"


TEST(test_LiveStreamIntegrationTest, StreamInsertAndRetrieveQuote)
{
    Log::init();
    
    // tastyworks
    std::unique_ptr<TastyWorksClient> twClient = std::make_unique<TastyWorksClient>();
    
    // streamer
    std::unique_ptr<DX_LinkStreamer> dxlStreamer = std::make_unique<DX_LinkStreamer>(
        *twClient
    );

    // strategy
    std::unique_ptr<MACD> strategy = std::make_unique<MACD>(
        /*fast=*/3, /*slow=*/6, /*signal=*/3
    );

    bool called = false;
    dxlStreamer->set_on_quote([&](const MarketQuote& quote)
    {
        Signal signal = strategy->generate_trading_signal(quote);
        called = true;
    });
    
    SUCCEED();
}

TEST(test_LiveStreamIntegrationTest, StreamCallsStrategyAndProducesSignals)
{
    MockStreamer streamer;

    std::unique_ptr<MACD> strategy = std::make_unique<MACD>(
        /*fast=*/3, /*slow=*/6, /*signal=*/3
    );

    int callbackCount = 0;
    bool sawBuy = false;

    streamer.set_on_quote([&](const MarketQuote& quote) {
        ++callbackCount;
        Signal s = strategy->generate_trading_signal(quote);
        sawBuy  |= (s == Signal::BUY);
    });

    std::vector<MarketQuote> quotes;

    for (int i = 0; i < 10; ++i) quotes.push_back(make_trade("BTC", 100.0));
    for (int i = 0; i < 30; ++i) quotes.push_back(make_trade("BTC", 100.0 - i));
    for (int i = 0; i < 40; ++i) quotes.push_back(make_trade("BTC", 70.0 + 2.0*i));

    streamer.emit(quotes);

    EXPECT_EQ(callbackCount, (int)quotes.size());
    EXPECT_TRUE(sawBuy);
}