#include <iostream>
#include <memory>
#include <stdexcept>
#include <exception>
#include <map>
#include <vector>
#include <string>
#include <csignal>

#include "tastyworks/TastyWorks.h"
#include "streamer/DX_LinkStreamer.h"
#include "marketquote/MarketQuote.h"
#include "log/Log.h"
#include "algo/vmacd/VMACD.h"
#include "algo/Signal.h"
#include "orchestrator/Orchestrator.h"

static DX_LinkStreamer* g_streamer = nullptr; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

static void handle_signal(int /*signum*/)
{
    if (g_streamer != nullptr)
        g_streamer->stop();
}

auto main(int argc, char** argv) -> int
{
    try 
    {
        Log::init();
        LOG_INFO("Starting mochaFiTrader.", "MAIN");
        
        LOG_INFO("Initialising TastyWorksClient object.", "MAIN");
        std::unique_ptr<TastyWorksClient> twClient = std::make_unique<TastyWorksClient>();
        
        LOG_INFO("Initialising DX_LinkStreamer object.", "MAIN");
        std::unique_ptr<DX_LinkStreamer> dxlStreamer = std::make_unique<DX_LinkStreamer>(*twClient);

        LOG_INFO("Initialising strategy object.", "MAIN");
        std::unique_ptr<VMACD> strategy = std::make_unique<VMACD>();

        LOG_INFO("Initialising Orchestrator object.", "MAIN");
        Orchestrator orch(*twClient, *dxlStreamer);

        g_streamer = dxlStreamer.get();
        std::signal(SIGINT, handle_signal);
        std::signal(SIGTERM, handle_signal);

        LOG_INFO("Triggering data stream.", "MAIN");
        dxlStreamer->set_on_quote([&](const MarketQuote& quote)
        {
            LOG_INFO("Received Trade Signal.", "MAIN");
            Signal signal = strategy->generate_trading_signal(quote);

            LOG_INFO("Conducting trade on signal.", "MAIN");
            orch.on_signal(signal);

        });
        dxlStreamer->run();
    } 
    
    catch (const std::invalid_argument& e)
    {
        std::string e_what(e.what());
        LOG_ERROR("Invalid Arg Error: " + e_what, "MAIN");
    }
    
    catch (const std::runtime_error& e)
    {
        std::string e_what(e.what());
        LOG_ERROR("Runtime Error: " + e_what, "MAIN");
    }
    
    catch (const std::exception& e)
    {
        std::string e_what(e.what());
        LOG_ERROR("Error: " + e_what, "MAIN");
    }

    Log::shutdown();

    return 0;
}