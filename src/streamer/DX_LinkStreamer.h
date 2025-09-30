#ifndef DX_LINKSTREAMER_H
#define DX_LINKSTREAMER_H

#include <string>
#include <iostream>
#include <nlohmann/json.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <boost/asio/ssl/context.hpp>
#include <chrono>
#include <thread>
#include <functional>
#include <mutex>

#include "TastyWorks.h"
#include "../marketquote/MarketQuote.hpp"
#include "../database/DB_Client.h"

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;

using websocketpp::connection_hdl;

class DX_LinkStreamer {
private:
    // required for class
    std::string instrument_type;
    std::string instrument;
    TastyWorksClient& twClient;
    DB_Client& dbClient;

    const int TIMEOUT = 5; // seconds
    const int SETUP_CHANNEL = 0;
    const int FEED_CHANNEL = 3;

    nlohmann::json setup_msg;
    nlohmann::json authorize_msg;
    nlohmann::json channel_request_msg;
    nlohmann::json feed_setup_msg;
    nlohmann::json feed_subscription_msg;
    nlohmann::json keep_alive_msg;

    // from TastyWorksClient class    
    std::string ws_url;
    std::string api_quote_token;

    void populate_class_attrs();
    
    void construct_setup_msg();
    void construct_authorize_msg();
    void construct_channel_request_msg();
    void construct_feed_setup_msg();
    void construct_feed_subscription_msg();
    void construct_keep_alive_msg();

    client c;
    connection_hdl conn_hdl;
    std::thread keep_alive_thread;

public:
    DX_LinkStreamer(
        const std::string& instrument_type, 
        const std::string& instrument, 
        TastyWorksClient& twClient,
        DB_Client& dbClient
    );

    ~DX_LinkStreamer();

    void send(const nlohmann::json& msg);
    void run();

    std::optional<double> safe_parse_quote(const nlohmann::json& pckt, const std::string& key);
};

#endif // DX_LINKSTREAMER_H