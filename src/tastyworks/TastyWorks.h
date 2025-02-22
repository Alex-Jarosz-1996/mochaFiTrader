#ifndef TASTYWORKSCLIENT_H
#define TASTYWORKSCLIENT_H

#include <string>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

class TastyWorksClient {
    private:
        std::string BASE_URL;
        std::string LOGIN;
        std::string PASSWORD;
        std::string ACCOUNT_NUMBER;
        bool REMEMBER_ME;
        std::string session_token;
    
        void loadConfig();
    
    public:
        TastyWorksClient();
    
        ~TastyWorksClient();
        
        std::string getSessionToken();
    };

#endif // TASTYWORKSCLIENT_H
