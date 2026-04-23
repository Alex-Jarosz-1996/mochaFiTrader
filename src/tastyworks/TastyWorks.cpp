#include "TastyWorks.h"

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include "src/config/Config.h"
#include "src/log/Log.h"
#include "src/utils/Utils.h"

TastyWorksClient::TastyWorksClient(bool auto_init)
{
    loadConfig();
    constructHeader();
    if (auto_init)
    {
        getSessionToken();          // NOLINT(clang-analyzer-optin.cplusplus.VirtualCall)
        constructAuthHeader();
        confirmUserAccountActive(); // NOLINT(clang-analyzer-optin.cplusplus.VirtualCall)
        defineQuoteTokenStreamUrl(); // NOLINT(clang-analyzer-optin.cplusplus.VirtualCall)
    }
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
    HTTP_TIMEOUT_MS = static_cast<int>(Config::get_numeric_value("HTTP_TIMEOUT_MS"));
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
    
    cpr::Response response = cpr::Delete(
        cpr::Url{session_url},
        _h_auth,
        cpr::Timeout{HTTP_TIMEOUT_MS}
    );

    if (Utils::isCorrectStatusCode(response, HTTP_NO_CONTENT))
    {
        LOG_INFO("Successfully logged out of user account.", "TASTYWORKS");
        return;
    }
    
    LOG_ERROR("Unable to log out of user account. Status Code: " + std::to_string(response.status_code) + " Reasoning: " + response.text, "TASTYWORKS");
}

auto TastyWorksClient::getBaseUrl() -> std::string
{
    return BASE_URL;
}

auto TastyWorksClient::getAccountNumber() -> std::string
{
    return ACCOUNT_NUMBER;
}

void TastyWorksClient::getSessionToken()
{
    LOG_INFO("Generating session token.", "TASTYWORKS");
    
    // checking for empty login credentials
    if (LOGIN.empty())
    {
        throw std::invalid_argument("Missing LOGIN account credentials. Check config.json file.");
    }

    if (PASSWORD.empty())
    {
        throw std::invalid_argument("Missing PASSWORD account credentials. Check config.json file.");
    }
    
    if (ACCOUNT_NUMBER.empty())
    {
        throw std::invalid_argument("Missing ACCOUNT_NUMBER account credentials. Check config.json file.");
    }
    
    std::string session_url = BASE_URL + "/sessions";

    nlohmann::json json_body = {
        {"login", LOGIN},
        {"password", PASSWORD},
        {"remember-me", REMEMBER_ME}
    };

    for (int attempt = 0; attempt < MAX_RETRIES; ++attempt)
    {
        cpr::Response response = cpr::Post(
            cpr::Url{session_url},
            _h,
            cpr::Body{json_body.dump()},
            cpr::Timeout{HTTP_TIMEOUT_MS}
        );

        if (Utils::isCorrectStatusCode(response, HTTP_CREATED))
        {
            LOG_INFO("Authentication successful. Retrieving session token.", "TASTYWORKS");

            nlohmann::json json_response = Utils::parseJsonResponse(response);

            _session_token = Utils::getJsonResponseAttrValue<std::string>(json_response, "session-token");

            LOG_INFO("Checking if session token was generated.", "TASTYWORKS");
            if (_session_token.empty())
            {
                throw std::invalid_argument("ERROR TastyWorksClient::getSessionToken(). No session token generated. Check login details.");
            }

            LOG_INFO("Session token was generated.", "TASTYWORKS");
            return;
        }

        LOG_WARN("Session token attempt " + std::to_string(attempt + 1) + " failed. Status: "
                 + std::to_string(response.status_code), "TASTYWORKS");

        if (attempt + 1 < MAX_RETRIES)
            std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_DELAY_MS));
    }

    throw std::invalid_argument("ERROR TastyWorksClient::getSessionToken(). Issue generating session token after "
                                + std::to_string(MAX_RETRIES) + " attempts.");
}

void TastyWorksClient::confirmUserAccountActive()
{
    LOG_INFO("Checking if user account is active.", "TASTYWORKS");
    
    std::string account_active_url = BASE_URL + "/customers/me/accounts/" + ACCOUNT_NUMBER;
    
    cpr::Response response = cpr::Get(
        cpr::Url{account_active_url},
        _h_auth,
        cpr::Timeout{HTTP_TIMEOUT_MS}
    );

    if (Utils::isCorrectStatusCode(response, HTTP_OK))
    {
        nlohmann::json json_response = Utils::parseJsonResponse(response);

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

        throw std::invalid_argument("ERROR TastyWorksClient::confirmUserAccountActive(). Json response does not contain 'data' attribute.");
    }

    throw std::invalid_argument("ERROR TastyWorksClient::confirmUserAccountActive(). Did not receive 200 response when checking if user account is active.");
}

void TastyWorksClient::defineQuoteTokenStreamUrl()
{
    LOG_INFO("Checking if can retrieve API quote token.", "TASTYWORKS");

    std::string api_quote_token_url = BASE_URL + "/api-quote-tokens";

    cpr::Response response = cpr::Get(
        cpr::Url{api_quote_token_url},
        _h_auth,
        cpr::Timeout{HTTP_TIMEOUT_MS}
    );

    if (Utils::isCorrectStatusCode(response, HTTP_OK))
    {
        nlohmann::json json_response = Utils::parseJsonResponse(response);
        
        if (Utils::doesJsonResponseContainDataAttr(json_response))
        {
            if (Utils::checkForAttrInsideJsonResponse(json_response, "dxlink-url") && Utils::checkForAttrInsideJsonResponse(json_response, "token"))
            {
                _dx_link_url = Utils::getJsonResponseAttrValue<std::string>(json_response, "dxlink-url");
                _api_quote_token = Utils::getJsonResponseAttrValue<std::string>(json_response, "token");

                return;    
            }
            
            throw std::invalid_argument("ERROR TastyWorksClient::defineQuoteTokenStreamUrl(). Json response does not contain either 'dxlink-url' or 'token' attribute.");
        }
        
        throw std::invalid_argument("ERROR TastyWorksClient::defineQuoteTokenStreamUrl(). Json response does not contain 'data' attribute.");
    }
    
    throw std::invalid_argument("ERROR TastyWorksClient::defineQuoteTokenStreamUrl(). Did not receive 200 response when retrieving feed token and Url.");
}

auto TastyWorksClient::getAPI_QuoteToken() -> std::string
{
    return _api_quote_token;
}

auto TastyWorksClient::getDX_LinkUrl() -> std::string
{
    return _dx_link_url;
}

auto TastyWorksClient::getAccountBalance() -> double
{
    LOG_INFO("Determining current account balance.", "TASTYWORKS");
    std::string account_active_url = BASE_URL + "/accounts/" + ACCOUNT_NUMBER + "/balances";
    
    cpr::Response response = cpr::Get(
        cpr::Url{account_active_url},
        _h_auth,
        cpr::Timeout{HTTP_TIMEOUT_MS}
    );

    if (Utils::isCorrectStatusCode(response, HTTP_OK))
    {
        nlohmann::json json_response = Utils::parseJsonResponse(response);

        if (Utils::doesJsonResponseContainDataAttr(json_response))
        {
            if (Utils::checkForAttrInsideJsonResponse(json_response, "net-liquidating-value"))
            {
                auto nlv = Utils::getJsonResponseAttrValue<std::string>(json_response, "net-liquidating-value");
                double nlv_d = std::stod(nlv);

                if (nlv_d < Config::get_numeric_value("ACCOUNT_MIN"))
                {
                    throw std::invalid_argument("Unable to trade as account is less than threshold.");
                }
                
                return nlv_d;
            }

            throw std::invalid_argument("ERROR TastyWorksClient::confirmAccountBalanceAdequate(). Json Response does not contain 'net-liquidating-value' attribute.");
        }
        
        throw std::invalid_argument("ERROR TastyWorksClient::confirmAccountBalanceAdequate(). Json response does not contain 'data' attribute.");
    }
    
    throw std::invalid_argument("ERROR TastyWorksClient::confirmAccountBalanceAdequate(). Did not receive 200 response when checking min account balance.");
}

auto TastyWorksClient::getTradeableAmount() -> double
{
    LOG_INFO("Determining tradeable amount.", "TASTYWORKS");
    return Config::get_numeric_value("TRADE_FACTOR") * getAccountBalance();
}

auto TastyWorksClient::getNumberAccountPositions() -> size_t
{
    LOG_INFO("Returning all open account positions.", "TASTYWORKS");
    std::string positions_url = BASE_URL + "/accounts/" + ACCOUNT_NUMBER + "/positions";

    cpr::Response response = cpr::Get(
        cpr::Url{positions_url},
        _h_auth,
        cpr::Timeout{HTTP_TIMEOUT_MS}
    );

    if (Utils::isCorrectStatusCode(response, HTTP_OK))
    {
        nlohmann::json json_response = Utils::parseJsonResponse(response);

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

    for (int attempt = 0; attempt < MAX_RETRIES; ++attempt)
    {
        cpr::Response response = cpr::Post(
            cpr::Url{submit_order_url},
            _h_auth,
            cpr::Body{body.dump()},
            cpr::Timeout{HTTP_TIMEOUT_MS}
        );

        if (Utils::isCorrectStatusCode(response, HTTP_OK) || Utils::isCorrectStatusCode(response, HTTP_CREATED))
        {
            return;
        }

        LOG_WARN("Order submission attempt " + std::to_string(attempt + 1) + " failed. Status: "
                 + std::to_string(response.status_code), "TASTYWORKS");

        if (attempt + 1 < MAX_RETRIES)
            std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_DELAY_MS));
    }

    throw std::invalid_argument("HTTP error submitting order after " + std::to_string(MAX_RETRIES) + " attempts.");
}