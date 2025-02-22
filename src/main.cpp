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

    } 
    catch (const std::exception& e)
    {
        std::cout << "Error: " << e.what() << std::endl;
    }

    return 0;
}