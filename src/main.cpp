#include <iostream>
#include <memory>
#include <stdexcept>
#include <exception>

#include "tastyworks/TastyWorks.h"

int main(int argc, char** argv)
{
    try 
    {
        std::unique_ptr<TastyWorksClient> twClient = std::make_unique<TastyWorksClient>();
        std::string session_token = twClient->getSessionToken();

        if (session_token == "") {
            throw std::invalid_argument("No session token generated. Check login details.");
        }

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