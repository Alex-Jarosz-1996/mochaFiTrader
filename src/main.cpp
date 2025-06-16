#include <iostream>
#include <memory>
#include <stdexcept>
#include <exception>

#include "tastyworks/TastyWorks.h"
#include "streamer/DX_LinkStreamer.h"

int main(int argc, char** argv)
{
    try 
    {
        // declarations
        std::string tradeable_assets = "BTC/USD:CXTALP";
        std::string instrument_type = "options";
        std::unique_ptr<TastyWorksClient> twClient = std::make_unique<TastyWorksClient>();
        if (twClient == nullptr)
        {
            throw std::runtime_error("twClient null pointer");
        }

        // Developer market data streamer
        std::unique_ptr<DX_LinkStreamer> dxlStreamer = std::make_unique<DX_LinkStreamer>(
            instrument_type, tradeable_assets, *twClient
        );
        if (dxlStreamer == nullptr)
        {
            throw std::runtime_error("dxlStreamer null pointer");
        }
        dxlStreamer->run();
        
        // Develop PostgreSQL db; read and write

        // Develop Algorithm class (consumes db data) : (VolumeWeightedMA)

        // Develop trade submission class
        // TODO: check whether make orders in seconds, minutes basis etc

        std::cout << "Successfully completed project workflow" << std::endl;

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