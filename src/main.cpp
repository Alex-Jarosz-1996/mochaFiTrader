#include <iostream>
#include <memory>
#include <stdexcept>
#include <exception>
#include <vector>

#include "tastyworks/TastyWorks.h"

int main(int argc, char** argv)
{
    try 
    {
        // Tradeable assets
        std::vector<std::string> tradeable_assets = {"BTC/USD", "ETH/USD", "LTC/USD"};

        // Generating token
        std::unique_ptr<TastyWorksClient> twClient = std::make_unique<TastyWorksClient>();
        twClient->confirmSessionTokenGenerated();

        // Check whether account is active, able to make trades
        twClient->confirmUserAccountActive();

        // Developer market data streamer
        // TODO: check market stream data in seconds, minutes etc

        // Develop PostgreSQL db; read and write

        // Develop Algorithm class (consumes db data) : (VolumeWeightedMA)

        // Develop trade submission class
        // TODO: check whether make orders in seconds, minutes basis etc

        std::cout << "Successfully completed project workflow" << std::endl;

    } 
    catch (const std::invalid_argument e)
    {
        std::cout << "Error: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << "Error: " << e.what() << std::endl;
    }

    return 0;
}