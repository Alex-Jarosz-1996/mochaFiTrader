#include "TastyWorks.h"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

TastyWorksClient::TastyWorksClient() {
    loadConfig();
}

TastyWorksClient::~TastyWorksClient() {}

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
        
        std::string session_token = json["data"]["session-token"];
        return session_token;
    }

    std::cerr << "Status Code: " << r.status_code << std::endl;
    std::cerr << "Reasoning: " << r.text << std::endl;
    return "";
}
