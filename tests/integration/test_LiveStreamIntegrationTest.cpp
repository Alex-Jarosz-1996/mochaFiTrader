#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>
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

    std::unique_ptr<TastyWorksClient> twClient;
    std::unique_ptr<DX_LinkStreamer>  dxlStreamer;
    try {
        twClient   = std::make_unique<TastyWorksClient>();
        dxlStreamer = std::make_unique<DX_LinkStreamer>(*twClient);
    } catch (const std::exception& e) {
        GTEST_SKIP() << "Skipping: unable to construct live dependencies. Reason: " << e.what();
    }

    static constexpr int MACD_FAST   = 3;
    static constexpr int MACD_SLOW   = 6;
    static constexpr int MACD_SIGNAL = 3;
    auto strategy = std::make_unique<MACD>(
        MACD::Params{.fast = MACD_FAST, .slow = MACD_SLOW, .signal = MACD_SIGNAL}
    );

    bool called = false;
    dxlStreamer->set_on_quote([&](const MarketQuote& quote)
    {
        strategy->generate_trading_signal(quote);
        called = true;
    });

    SUCCEED();
}

TEST(test_LiveStreamIntegrationTest, StreamCallsStrategyAndProducesSignals)
{
    MockStreamer dxlStreamer;

    static constexpr int    MACD_FAST2      = 3;
    static constexpr int    MACD_SLOW2      = 6;
    static constexpr int    MACD_SIGNAL2    = 3;
    static constexpr int    WARMUP_ITERS    = 10;
    static constexpr int    DOWNTREND_ITERS = 30;
    static constexpr int    UPTREND_ITERS   = 40;
    static constexpr double SEED_PRICE      = 100.0;
    static constexpr double DOWN_BASE       = 100.0;
    static constexpr double UP_BASE         = 70.0;
    static constexpr double UP_STEP         = 2.0;

    std::unique_ptr<MACD> strategy = std::make_unique<MACD>(
        MACD::Params{.fast = MACD_FAST2, .slow = MACD_SLOW2, .signal = MACD_SIGNAL2}
    );

    int callbackCount = 0;
    bool sawBuy = false;

    dxlStreamer.set_on_quote([&](const MarketQuote& quote) {
        ++callbackCount;
        Signal sig = strategy->generate_trading_signal(quote);
        sawBuy |= (sig == Signal::BUY);
    });

    std::vector<MarketQuote> quotes;
    quotes.reserve(WARMUP_ITERS + DOWNTREND_ITERS + UPTREND_ITERS);

    for (int i = 0; i < WARMUP_ITERS; ++i)    quotes.push_back(make_trade("BTC", SEED_PRICE));
    for (int i = 0; i < DOWNTREND_ITERS; ++i) quotes.push_back(make_trade("BTC", DOWN_BASE - i));
    for (int i = 0; i < UPTREND_ITERS; ++i)   quotes.push_back(make_trade("BTC", UP_BASE + UP_STEP * i));

    dxlStreamer.emit(quotes);

    EXPECT_EQ(callbackCount, (int)quotes.size());
    EXPECT_TRUE(sawBuy);
}

TEST(test_LiveStreamIntegrationTest, LiveStreamProbe)
{
    try
    {
        Log::init();
    
        // tastyworks
        std::unique_ptr<TastyWorksClient> twClient = std::make_unique<TastyWorksClient>();
        
        // streamer
        std::unique_ptr<DX_LinkStreamer> dxlStreamer = std::make_unique<DX_LinkStreamer>(
            *twClient
        );

        // quote
        std::atomic<int> quote_count{0};
        std::atomic<bool> got_quote{false};
        const int THRESHOLD{5};

        dxlStreamer->set_on_quote([&](const MarketQuote& quote)
        {
            if (!quote.symbol.empty())
            {
                got_quote = true;
                ++quote_count;
                if (quote_count.load() >= THRESHOLD)
                {
                    std::exit(0);
                }
            }
        });

        std::thread runner([&](){
            dxlStreamer->run(); // blocks
        });

        static constexpr int POLL_INTERVAL_MS = 50;
        static constexpr int TIMEOUT_SECS     = 12;

        auto start = std::chrono::steady_clock::now();
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(POLL_INTERVAL_MS));
            if ((std::chrono::steady_clock::now() - start) > std::chrono::seconds(TIMEOUT_SECS))
            {
                std::cerr << "Timeout waiting for quotes. Got "
                          << quote_count.load() << "\n";
                std::exit(2);
            }
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}