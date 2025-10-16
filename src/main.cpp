#include <iostream>
#include <memory>
#include <stdexcept>
#include <exception>

#include "tastyworks/TastyWorks.h"
#include "streamer/DX_LinkStreamer.h"
#include "database/DB_Client.h"
#include "marketquote/MarketQuote.hpp"

int main(int argc, char** argv)
{
    try 
    {
        // database object
        std::unique_ptr<DB_Client> dbClient = std::make_unique<DB_Client>();
        
        // account details
        std::unique_ptr<TastyWorksClient> twClient = std::make_unique<TastyWorksClient>();
        
        // market data streamer
        std::unique_ptr<DX_LinkStreamer> dxlStreamer = std::make_unique<DX_LinkStreamer>(
            *twClient
        );
        dxlStreamer->set_on_quote([&](const MarketQuote& quote)
        {
            dbClient->insert_quote(quote);
        });
        dxlStreamer->run();

        // Develop Algorithm class (consumes db data) : (VolumeWeightedMA)

        // Develop trade submission class
        // TODO: check whether make orders in seconds, minutes basis etc

    } 
    
    catch (const std::invalid_argument& e)
    {
        std::cout << "Invalid Arg Error: " << e.what() << std::endl;
    }
    
    catch (const std::runtime_error& e)
    {
        std::cout << "Runtime Error: " << e.what() << std::endl;
    }
    
    catch (const std::exception& e)
    {
        std::cout << "Error: " << e.what() << std::endl;
    }

    return 0;
}