#include <iostream>
#include <memory>
#include <stdexcept>
#include <exception>
#include <map>
#include <vector>
#include <string>

#include "tastyworks/TastyWorks.h"
#include "streamer/DX_LinkStreamer.h"
#include "database/DB_Client.h"
#include "marketquote/MarketQuote.hpp"
#include "log/Log.h"

int main(int argc, char** argv)
{
    try 
    {
        // logger initialisation
        Log::init();
        LOG_INFO("Starting mochaFiTrader.", "MAIN");
        
        LOG_INFO("Initialising DB_Client object.", "MAIN");
        std::unique_ptr<DB_Client> dbClient = std::make_unique<DB_Client>();
        
        LOG_INFO("Initialising TastyWorksClient object.", "MAIN");
        std::unique_ptr<TastyWorksClient> twClient = std::make_unique<TastyWorksClient>();
        
        LOG_INFO("Initialising DX_LinkStreamer object.", "MAIN");
        std::unique_ptr<DX_LinkStreamer> dxlStreamer = std::make_unique<DX_LinkStreamer>(
            *twClient
        );

        LOG_INFO("Triggering data stream.", "MAIN");
        dxlStreamer->set_on_quote([&](const MarketQuote& quote)
        {
            LOG_FILE_ONLY("Writing quote to db.", "MAIN");
            dbClient->insert_quote(quote);
            
            LOG_FILE_ONLY("Getting quote from db.", "MAIN");
            std::optional<std::vector<MarketQuote>> quote_from_db = dbClient->get_quote();
        });
        dxlStreamer->run();
        
        // configure tests to work

        // Develop Algorithm class (consumes db data) : (VolumeWeightedMA)

        // Develop trade submission class
        // TODO: check whether make orders in seconds, minutes basis etc

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

    return 0;
}