#include <stdexcept>
#include <exception>

#include "DX_LinkStreamer.h"
#include "../marketquote/MarketQuote.hpp"
#include "../database/DB_Client.h"
#include "../config/Config.h"

DX_LinkStreamer::DX_LinkStreamer(
    TastyWorksClient& twClient
) : twClient(twClient)
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
    instrument_type = Config::get_config_value("INSTRUMENT_TYPE");
    tradeable_assets = Config::get_config_value("TRADEABLE_ASSETS");
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
            {{"type", "Trade"}, {"symbol", tradeable_assets}},
            {{"type", "Quote"}, {"symbol", tradeable_assets}}
        }}
    };

    if (instrument_type == "options") {
        feed_subscription_msg["add"].push_back(
            {{"type", "Greeks"}, {"symbol", tradeable_assets}}
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

void DX_LinkStreamer::set_on_quote(QuoteCallback qcb)
{
    on_quote = std::move(qcb);
}

void DX_LinkStreamer::send(const nlohmann::json& msg)
{
    c.send(conn_hdl, msg.dump(), websocketpp::frame::opcode::text);
    // std::cout << "Sent: " << msg.dump() << std::endl;
}

void DX_LinkStreamer::configure_channels()
{
    // suppressing logging output of frame headers and payload
    c.clear_access_channels(websocketpp::log::alevel::all);
    c.clear_error_channels(websocketpp::log::elevel::all);
    
    c.init_asio();
}

void DX_LinkStreamer::send_initial_msgs()
{
    send(setup_msg);
    send(authorize_msg);
    send(channel_request_msg);
    send(feed_setup_msg);
}

void DX_LinkStreamer::send_periodic_msgs()
{
    send(feed_subscription_msg);
    send(keep_alive_msg);
}

MarketQuote DX_LinkStreamer::parse_quote(const nlohmann::json& entry)
{
    MarketQuote q;
    q.symbol = entry.value("eventSymbol", "");
    q.price = safe_parse_quote(entry, "price");
    q.bidPrice = safe_parse_quote(entry, "bidPrice");
    q.askPrice = safe_parse_quote(entry, "askPrice");
    q.dayVolume = safe_parse_quote(entry, "dayVolume");
    q.size = safe_parse_quote(entry, "size");
    q.bidSize = safe_parse_quote(entry, "bidSize");
    q.askSize = safe_parse_quote(entry, "askSize");
    return q;
}

void DX_LinkStreamer::on_open(connection_hdl hdl)
{
    bool running = true;

    conn_hdl = hdl;
    send_initial_msgs();

    keep_alive_thread = std::thread([this]() {
        while (true) {
            
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            send_periodic_msgs();
        }
    });
}

void DX_LinkStreamer::on_msg(client::message_ptr msg)
{
    try
    {
        nlohmann::json pckt = nlohmann::json::parse(msg->get_payload());

        if (pckt["type"] == "FEED_DATA" && pckt.contains("data"))
        {
            for (const auto& entry : pckt["data"])
            {
                MarketQuote quote = parse_quote(entry);
                if (on_quote) on_quote(quote);
            }
        }
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(e.what());
    }
}

std::shared_ptr<boost::asio::ssl::context> DX_LinkStreamer::create_tls_context()
{
    auto ctx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);
    ctx->set_options(boost::asio::ssl::context::default_workarounds |
                     boost::asio::ssl::context::no_sslv2 |
                     boost::asio::ssl::context::no_sslv3 |
                     boost::asio::ssl::context::single_dh_use);
    return ctx;   
}

void DX_LinkStreamer::setup_handlers()
{
    c.set_open_handler([this](connection_hdl hdl)
    {
        on_open(hdl);
    });

    c.set_message_handler([this](connection_hdl, client::message_ptr msg)
    {
        on_msg(msg);
    });

    // tls init handler for secure connections
    c.set_tls_init_handler([this](connection_hdl)
    {
        return create_tls_context();
    });
}

client::connection_ptr DX_LinkStreamer::create_connection()
{
    websocketpp::lib::error_code ec;
    client::connection_ptr con = c.get_connection(ws_url, ec);
    
    if (ec) 
    {
        throw std::runtime_error("Connection initialization error: " + ec.message());
    }

    return con;
}

void DX_LinkStreamer::run()
{
    configure_channels();
    setup_handlers();

    // create connection
    client::connection_ptr con = create_connection();

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
