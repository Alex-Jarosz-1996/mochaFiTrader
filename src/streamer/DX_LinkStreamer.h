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
#include <atomic>

#include "../marketquote/MarketQuote.h"
#include "../tastyworks/TastyWorks.h"

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;

using websocketpp::connection_hdl;

class DX_LinkStreamer {
    public:
        using QuoteCallback = std::function<void(const MarketQuote&)>;

    private:
        TastyWorksClient& client_;
    
        QuoteCallback on_quote;

        std::string instrument_type;
        std::string tradeable_assets;

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

        void send(const nlohmann::json& msg);
        void configure_channels();
        void send_initial_msgs();
        void send_periodic_msgs();
        MarketQuote parse_quote(const nlohmann::json& entry);
        void on_open(connection_hdl hdl);
        void on_msg(client::message_ptr msg);
        std::shared_ptr<boost::asio::ssl::context> create_tls_context();
        void setup_handlers();
        client::connection_ptr create_connection();
        std::optional<double> safe_parse_quote(const nlohmann::json& pckt, const std::string& key);

    public:
        DX_LinkStreamer(TastyWorksClient& client);
        virtual ~DX_LinkStreamer();

        virtual void set_on_quote(QuoteCallback qcb);
        virtual void run();
};

#endif // DX_LINKSTREAMER_H