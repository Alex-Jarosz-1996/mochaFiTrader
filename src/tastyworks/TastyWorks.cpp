#include "TastyWorks.h"

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include "../config/Config.h"
#include "../log/Log.h"
#include "../utils/Utils.h"

TastyWorksClient::TastyWorksClient()
{
    loadConfig();
    constructHeader();
    getSessionToken();
    constructAuthHeader();
    confirmUserAccountActive();
    defineQuoteTokenStreamUrl();
}

TastyWorksClient::~TastyWorksClient()
{
    logout();
}

void TastyWorksClient::loadConfig()
{
    BASE_URL = Config::get_config_value("TASTYWORKS_BASE_URL");
    LOGIN = Config::get_config_value("TASTYWORKS_LOGIN");
    PASSWORD = Config::get_config_value("TASTYWORKS_PASSWORD");
    ACCOUNT_NUMBER = Config::get_config_value("TASTYWORKS_ACCOUNT_NUMBER");
    REMEMBER_ME = true;
}

void TastyWorksClient::constructHeader()
{
    _h = {
        {"User-Agent", "tastytrade-api-client/1.0"},
        {"Content-Type", "application/json"},
        {"Accept", "application/json"}
    };
}

void TastyWorksClient::constructAuthHeader()
{
    _h_auth = {
        {"User-Agent", "tastytrade-api-client/1.0"},
        {"Content-Type", "application/json"},
        {"Accept", "application/json"},
        {"Authorization", _session_token}
    };
}

void TastyWorksClient::logout()
{
    LOG_INFO("Logging out of current session.", "TASTYWORKS");
    
    std::string session_url = BASE_URL + "/sessions";
    
    cpr::Response r = cpr::Delete(
        cpr::Url{session_url},
        _h_auth,
        cpr::Timeout{5000}
    );

    if (Utils::isCorrectStatusCode(r, 204))
    {
        std::cout << "Successfully logged out of user account." << std::endl;
        return;
    }
    
    std::cout << "Unable to log out of user account." << std::endl;
    std::cerr << "Status Code: " << r.status_code << std::endl;
    std::cerr << "Reasoning: " << r.text << std::endl;
    return;
}

std::string TastyWorksClient::getBaseUrl()
{
    return BASE_URL;
}

std::string TastyWorksClient::getAccountNumber()
{
    return ACCOUNT_NUMBER;
}

void TastyWorksClient::getSessionToken()
{
    LOG_INFO("Generating session token.", "TASTYWORKS");
    
    // checking for empty login credentials
    if (LOGIN.empty())
    {
        std::cerr << "Missing LOGIN account credentials. Check config.json file." << std::endl;
        return;
    }

    if (PASSWORD.empty())
    {
        std::cerr << "Missing PASSWORD account credentials. Check config.json file." << std::endl;
        return;
    }
    
    if (ACCOUNT_NUMBER.empty())
    {
        std::cerr << "Missing ACCOUNT_NUMBER account credentials. Check config.json file." << std::endl;
        return;
    }
    
    std::string session_url = BASE_URL + "/sessions";

    nlohmann::json json_body = {
        {"login", LOGIN},
        {"password", PASSWORD},
        {"remember-me", REMEMBER_ME}
    };

    cpr::Response r = cpr::Post(
        cpr::Url{session_url},
        _h,
        cpr::Body{json_body.dump()},
        cpr::Timeout{5000}
    );

    if (Utils::isCorrectStatusCode(r, 201))
    {
        LOG_INFO("Successfully logged into user account.", "TASTYWORKS");

        nlohmann::json json_response = Utils::parseJsonResponse(r);
        
        _session_token = Utils::getJsonResponseAttrValue<std::string>(json_response, "session-token");

        LOG_INFO("Checking if session token was generated.", "TASTYWORKS");
        if (_session_token == "")
        {
            throw std::invalid_argument("ERROR TastyWorksClient::getSessionToken(). No session token generated. Check login details.");
        }

        LOG_INFO("Session token was generated.", "TASTYWORKS");
        return;
    }

    std::cerr << "Status Code: " << r.status_code << std::endl;
    std::cerr << "Reasoning: " << r.text << std::endl;
    throw std::invalid_argument("ERROR TastyWorksClient::getSessionToken(). Issue generating session token." + r.text);
}

void TastyWorksClient::confirmUserAccountActive()
{
    LOG_INFO("Checking if user account is active.", "TASTYWORKS");
    
    std::string account_active_url = BASE_URL + "/customers/me/accounts/" + ACCOUNT_NUMBER;
    
    cpr::Response r = cpr::Get(
        cpr::Url{account_active_url},
        _h_auth,
        cpr::Timeout{5000}
    );

    if (Utils::isCorrectStatusCode(r, 200))
    {
        nlohmann::json json_response = Utils::parseJsonResponse(r);

        if (Utils::doesJsonResponseContainDataAttr(json_response))
        {
            if (Utils::checkForAttrInsideJsonResponse(json_response, "is-closed"))
            {
                bool is_inactive = Utils::getJsonResponseAttrValue<bool>(json_response, "is-closed");
                
                // active = 1 / inactive = 0
                if (!is_inactive)
                {
                    LOG_INFO("User account is active.", "TASTYWORKS");
                    return;
                }
                
                throw std::invalid_argument("ERROR TastyWorksClient::confirmUserAccountActive(). User account is inactive. Please investigate.");
            }
            
            throw std::invalid_argument("ERROR TastyWorksClient::confirmUserAccountActive(). Json Response does not contain 'is-closed' attribute.");
        }

        throw std::invalid_argument("ERROR TastyWorksClient::confirmUserAccountActive(). Json response does contain 'data' attribute.");
    }

    throw std::invalid_argument("ERROR TastyWorksClient::confirmUserAccountActive(). Did not receive 200 response when checking if user account is active.");
}

void TastyWorksClient::defineQuoteTokenStreamUrl()
{
    LOG_INFO("Checking if can retrieve API quote token.", "TASTYWORKS");

    std::string api_quote_token_url = BASE_URL + "/api-quote-tokens";

    cpr::Response r = cpr::Get(
        cpr::Url{api_quote_token_url},
        _h_auth,
        cpr::Timeout{5000}
    );

    if (Utils::isCorrectStatusCode(r, 200))
    {
        nlohmann::json json_response = Utils::parseJsonResponse(r);
        
        if (Utils::doesJsonResponseContainDataAttr(json_response))
        {
            if (Utils::checkForAttrInsideJsonResponse(json_response, "dxlink-url") && Utils::checkForAttrInsideJsonResponse(json_response, "token"))
            {
                _dx_link_url = Utils::getJsonResponseAttrValue<std::string>(json_response, "dxlink-url");
                _api_quote_token = Utils::getJsonResponseAttrValue<std::string>(json_response, "token");

                return;    
            }
            
            throw std::invalid_argument("ERROR TastyWorksClient::defineQuoteTokenStreamUrl(). Json response does contain either 'dxlink-url' or 'token' attribute.");
        }
        
        throw std::invalid_argument("ERROR TastyWorksClient::defineQuoteTokenStreamUrl(). Json response does contain 'data' attribute.");
    }
    
    throw std::invalid_argument("ERROR TastyWorksClient::defineQuoteTokenStreamUrl(). Did not receive 200 response when retrieving feed token and Url.");
}

std::string TastyWorksClient::getAPI_QuoteToken()
{
    return _api_quote_token;
}

std::string TastyWorksClient::getDX_LinkUrl()
{
    return _dx_link_url;
}

double TastyWorksClient::getAccountBalance()
{
    LOG_INFO("Determining current account balance.", "TASTYWORKS");
    std::string account_active_url = BASE_URL + "/accounts/" + ACCOUNT_NUMBER + "/balances";
    
    cpr::Response r = cpr::Get(
        cpr::Url{account_active_url},
        _h_auth,
        cpr::Timeout{5000}
    );

    if (Utils::isCorrectStatusCode(r, 200))
    {
        nlohmann::json json_response = Utils::parseJsonResponse(r);

        if (Utils::doesJsonResponseContainDataAttr(json_response))
        {
            if (Utils::checkForAttrInsideJsonResponse(json_response, "net-liquidating-value"))
            {
                std::string nlv = Utils::getJsonResponseAttrValue<std::string>(json_response, "net-liquidating-value");
                double nlv_d = std::stod(nlv);

                // if (nlv_d < ACCOUNT_MIN)
                // {
                //     throw std::invalid_argument("Unable to trade as account is less than threshold.");
                // }
                
                return nlv_d;
            }

            throw std::invalid_argument("ERROR TastyWorksClient::confirmAccountBalanceAdequate(). Json Response does not contain 'net-liquidating-value' attribute.");
        }
        
        throw std::invalid_argument("ERROR TastyWorksClient::confirmAccountBalanceAdequate(). Json response does contain 'data' attribute.");
    }
    
    throw std::invalid_argument("ERROR TastyWorksClient::confirmAccountBalanceAdequate(). Did not receive 200 response when checking min account balance.");
}

double TastyWorksClient::getTradeableAmount()
{
    LOG_INFO("Determining tradeable amount.", "TASTYWORKS");
    return TRADE_FACTOR * getAccountBalance();
}

size_t TastyWorksClient::getNumberAccountPositions()
{
    LOG_INFO("Returning all open account positions.", "TASTYWORKS");
    std::string positions_url = BASE_URL + "/accounts/" + ACCOUNT_NUMBER + "/positions";

    cpr::Response r = cpr::Get(
        cpr::Url{positions_url},
        _h_auth,
        cpr::Timeout{5000}
    );

    if (Utils::isCorrectStatusCode(r, 200))
    {
        nlohmann::json json_response = Utils::parseJsonResponse(r);

        if (Utils::doesJsonResponseContainDataAttr(json_response))
        {
            if (Utils::checkForAttrInsideJsonResponse(json_response, "items"))
            {
                return Utils::getJsonResponseAttrArraySize(json_response, "items");
            }

            throw std::invalid_argument("ERROR TastyWorksClient::getNumberAccountPositions(). Json Response does not contain 'items' attribute.");
        }

        throw std::invalid_argument("ERROR TastyWorksClient::getNumberAccountPositions(). Json response does contain 'data' attribute.");
    }
    
    throw std::invalid_argument("ERROR TastyWorksClient::getNumberAccountPositions(). Did not receive 200 response when determining number of account positions active.");
}


void TastyWorksClient::submitOrder(const nlohmann::json& body)
{
    LOG_INFO("Submitting order.", "TASTYWORKS");
    std::string submit_order_url = BASE_URL + "/accounts/" + ACCOUNT_NUMBER + "/orders";
    
    cpr::Response r = cpr::Post(
        cpr::Url{submit_order_url},
        _h_auth,
        cpr::Body{body.dump()},
        cpr::Timeout{5000}
    );

    if (Utils::isCorrectStatusCode(r, 200) || Utils::isCorrectStatusCode(r, 201))
    {
        return;
    }

    throw std::invalid_argument("HTTP error submitting order: " + r.error.message);
}