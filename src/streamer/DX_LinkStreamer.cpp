#include <stdexcept>
#include <exception>

#include "DX_LinkStreamer.h"
#include "../marketquote/MarketQuote.hpp"
#include "../database/DB_Client.h"

DX_LinkStreamer::DX_LinkStreamer(
    const std::string& instrument_type, 
    const std::string& instrument, 
    TastyWorksClient& twClient,
    DB_Client& dbClient
) : instrument_type(instrument_type), instrument(instrument), twClient(twClient), dbClient(dbClient)
{
    // define appropriate feed parameters
    populate_class_attrs();

    // declare feed messages
    construct_setup_msg();
    construct_authorize_msg();
    construct_channel_request_msg();
    construct_feed_setup_msg();
    construct_feed_subscription_msg();
    construct_keep_alive_msg();
}

DX_LinkStreamer::~DX_LinkStreamer() {}

void DX_LinkStreamer::populate_class_attrs()
{
    ws_url = twClient._dx_link_url;
    api_quote_token = twClient._api_quote_token;
}

void DX_LinkStreamer::construct_setup_msg()
{
    setup_msg = nlohmann::json{
        {"type", "SETUP"},
        {"channel", SETUP_CHANNEL},
        {"version", "0.1-DXF-JS/0.3.0"},
        {"keepaliveTimeout", TIMEOUT},
        {"acceptKeepaliveTimeout", TIMEOUT}
    };
}

void DX_LinkStreamer::construct_authorize_msg()
{
    authorize_msg = nlohmann::json{
        {"type", "AUTH"},
        {"channel", SETUP_CHANNEL},
        {"token", api_quote_token}
    };
}

void DX_LinkStreamer::construct_channel_request_msg()
{
    channel_request_msg = nlohmann::json{
        {"type", "CHANNEL_REQUEST"},
        {"channel", FEED_CHANNEL},
        {"service", "FEED"},
        {"parameters", {{"contract", "AUTO"}}}
    };
}

void DX_LinkStreamer::construct_feed_setup_msg()
{
    feed_setup_msg = nlohmann::json{
        {"type", "FEED_SETUP"},
        {"channel", FEED_CHANNEL},
        {"acceptAggregationPeriod", 0.1},
        {"acceptDataFormat", "FEED_SETUP"},
        // {"acceptDataFormat", "COMPACT"},
        {"acceptEventFields", {
            {"Trade", {"eventSymbol", "price", "dayVolume", "size"}},
            {"Quote", {"eventSymbol", "bidPrice", "askPrice", "bidSize", "askSize"}}
        }}
    };

    if (instrument_type == "options") {
        feed_setup_msg["acceptEventFields"]["Greeks"] = {
            "eventSymbol", "volatility", "delta", "gamma", "theta", "rho", "vega"
        };
    }
}

void DX_LinkStreamer::construct_feed_subscription_msg()
{
    feed_subscription_msg = nlohmann::json{
        {"type", "FEED_SUBSCRIPTION"},
        {"channel", FEED_CHANNEL},
        {"reset", true},
        {"add", {
            {{"type", "Trade"}, {"symbol", instrument}},
            {{"type", "Quote"}, {"symbol", instrument}}
        }}
    };

    if (instrument_type == "options") {
        feed_subscription_msg["add"].push_back(
            {{"type", "Greeks"}, {"symbol", instrument}}
        );
    }
}

void DX_LinkStreamer::construct_keep_alive_msg()
{
    keep_alive_msg = nlohmann::json{
        {"type", "KEEPALIVE"},
        {"channel", FEED_CHANNEL}
    };
}

void DX_LinkStreamer::send(const nlohmann::json& msg)
{
    c.send(conn_hdl, msg.dump(), websocketpp::frame::opcode::text);
    // std::cout << "Sent: " << msg.dump() << std::endl;
}

void DX_LinkStreamer::run()
{
    // suppressing logging output of frame headers and payload
    c.clear_access_channels(websocketpp::log::alevel::all);
    c.clear_error_channels(websocketpp::log::elevel::all);
    
    c.init_asio();

    // connection opened handler
    c.set_open_handler([this](connection_hdl hdl) {
        conn_hdl = hdl;
        send(setup_msg);
        send(authorize_msg);
        send(channel_request_msg);
        send(feed_setup_msg);

        // Start periodic keep-alive and subscription
        keep_alive_thread = std::thread([this]() {
            while (true) {
                
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                send(feed_subscription_msg);
                send(keep_alive_msg);
            }
        });
    });

    // message handler
    c.set_message_handler([this](connection_hdl, client::message_ptr msg) {
        // std::cout << "Received: " << msg->get_payload() << std::endl;
        try
        {
            nlohmann::json pckt = nlohmann::json::parse(msg->get_payload());

            if (pckt["type"] == "FEED_DATA" && pckt.contains("data"))
            {
                for (const auto& entry : pckt["data"])
                {
                    MarketQuote quote;

                    quote.symbol = entry.value("eventSymbol", "");
                    
                    quote.price = safe_parse_quote(entry, "price");
                    quote.bidPrice = safe_parse_quote(entry, "bidPrice");
                    quote.askPrice = safe_parse_quote(entry, "askPrice");
                    quote.dayVolume = safe_parse_quote(entry, "dayVolume");
                    quote.size = safe_parse_quote(entry, "size");
                    quote.bidSize = safe_parse_quote(entry, "bidSize");
                    quote.askSize = safe_parse_quote(entry, "askSize");

                    dbClient.insert_quote(quote);
                    
                    // quote.print_summary();
                }
            }
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error(e.what());
        }
    });

    // tls init handler for secure connections
    c.set_tls_init_handler([](websocketpp::connection_hdl) {
        auto ctx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
                        boost::asio::ssl::context::no_sslv2 |
                        boost::asio::ssl::context::no_sslv3 |
                        boost::asio::ssl::context::single_dh_use);
        return ctx;
    });

    // create connection
    websocketpp::lib::error_code ec;
    client::connection_ptr con = c.get_connection(ws_url, ec);
    
    if (ec) 
    {
        throw std::runtime_error("Connection initialization error: " + ec.message());
    }

    c.connect(con);

    c.run();
}

std::optional<double> DX_LinkStreamer::safe_parse_quote(const nlohmann::json& pckt, const std::string& key)
{
    if (pckt.contains(key) && pckt[key].is_number()) {
        return pckt[key].get<double>();
    }
    return std::nullopt;
}
