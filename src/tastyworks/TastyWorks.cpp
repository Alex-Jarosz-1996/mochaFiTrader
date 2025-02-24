#include "TastyWorks.h"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

TastyWorksClient::TastyWorksClient() {
    loadConfig();
    _session_token = TastyWorksClient::getSessionToken();
}

TastyWorksClient::~TastyWorksClient() {
    logout();
}

void TastyWorksClient::logout() {
    std::string session_url = BASE_URL + "/sessions";
    
    cpr::Header headers = {
        {"User-Agent", "tastytrade-api-client/1.0"},
        {"Content-Type", "application/json"},
        {"Accept", "application/json"},
        {"Authorization", _session_token}
    };
    
    cpr::Response r = cpr::Delete(cpr::Url{session_url},
        headers,
        cpr::Timeout{5000}
    );

    if (r.status_code == 204) {
        std::cout << "Successfully logged out of user account." << std::endl;
        return;
    }
    
    std::cout << "Unable to log out of user account." << std::endl;
    std::cerr << "Status Code: " << r.status_code << std::endl;
    std::cerr << "Reasoning: " << r.text << std::endl;
    return;
}

void TastyWorksClient::loadConfig() {
    std::ifstream file("config.json");
    if (!file.is_open()) {
        std::cerr << "Could not open mochaFiTrader config.json" << std::endl;
        return;
    }

    nlohmann::json config;
    file >> config;
    file.close();

    BASE_URL = config.value("TASTYWORKS_BASE_URL", "");
    LOGIN = config.value("TASTYWORKS_LOGIN", "");
    PASSWORD = config.value("TASTYWORKS_PASSWORD", "");
    ACCOUNT_NUMBER = config.value("TASTYWORKS_ACCOUNT_NUMBER", "");
    REMEMBER_ME = true;
}

std::string TastyWorksClient::getSessionToken() {
    // checking for empty login credentials
    if (LOGIN.empty() || PASSWORD.empty() || ACCOUNT_NUMBER.empty()) {
        std::cerr << "Missing account credentials. Check config.json file." << std::endl;
        return "";
    }
    
    std::string session_url = BASE_URL + "/sessions";

    cpr::Header headers = {
        {"User-Agent", "tastytrade-api-client/1.0"},
        {"Content-Type", "application/json"},
        {"Accept", "application/json"}
    };

    nlohmann::json json_body = {
        {"login", LOGIN},
        {"password", PASSWORD},
        {"remember-me", REMEMBER_ME}
    };

    cpr::Response r = cpr::Post(cpr::Url{session_url},
        headers,
        cpr::Body{json_body.dump()},
        cpr::Timeout{5000}
    );

    if (r.status_code == 201) {
        std::cout << "Successfully logged into user account." << std::endl;

        auto json = nlohmann::json::parse(r.text);
        
        std::string _session_token = json["data"]["session-token"];
        return _session_token;
    }

    std::cerr << "Status Code: " << r.status_code << std::endl;
    std::cerr << "Reasoning: " << r.text << std::endl;
    return "";
}


void TastyWorksClient::confirmSessionTokenGenerated() {
    std::cout << "Checking if session token was generated." << std::endl;
    
    if (TastyWorksClient::_session_token == "") {
        throw std::invalid_argument("No session token generated. Check login details.");
    }

    std::cout << "Session token was generated." << std::endl;
}

void TastyWorksClient::confirmAccountActive() {
    std::cout << "Checking if user account is active." << std::endl;
    
    std::string account_active_url = BASE_URL + "/customers/me/accounts/" + ACCOUNT_NUMBER;

    cpr::Header headers = {
        {"User-Agent", "tastytrade-api-client/1.0"},
        {"Content-Type", "application/json"},
        {"Accept", "application/json"},
        {"Authorization", _session_token}
    };
    
    cpr::Response r = cpr::Get(cpr::Url{account_active_url},
        headers,
        cpr::Timeout{5000}
    );

    if (r.status_code == 200) {
        nlohmann::json json_response = nlohmann::json::parse(r.text);
        
        if (json_response.contains("data") && json_response["data"].contains("is-closed")) {
            bool is_inactive = json_response["data"]["is-closed"];
            
            // active = 1 / inactive = 0
            if (!is_inactive) {
                std::cout << "User account is active." << std::endl;
                return;
            }
            
            throw std::invalid_argument("User account is inactive. Please investigate.");
        }

        throw std::invalid_argument("User Account Get Request response bpdy is not of appropriate format.");
    }

    throw std::invalid_argument("Did not receive 200 response when checking if user account is active.");
}
