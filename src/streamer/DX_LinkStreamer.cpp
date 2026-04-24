#include <stdexcept>
#include <exception>

#include "DX_LinkStreamer.h"
#include "src/marketquote/MarketQuote.h"
#include "src/config/Config.h"
#include "src/log/Log.h"
#include "src/tastyworks/TastyWorks.h"

DX_LinkStreamer::DX_LinkStreamer(TastyWorksClient& client)
    : client_(client)
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

DX_LinkStreamer::~DX_LinkStreamer()
{
    stop(); // NOLINT(clang-analyzer-optin.cplusplus.VirtualCall)
    if (keep_alive_thread.joinable())
        keep_alive_thread.join();
}

void DX_LinkStreamer::stop()
{
    stop_flag_.store(true);
    if (running_.load())
        c.stop();
}

void DX_LinkStreamer::populate_class_attrs()
{
    instrument_type = Config::get_config_value("INSTRUMENT_TYPE");
    tradeable_assets = Config::get_config_value("TRADEABLE_ASSETS");
    
    ws_url = client_.getDX_LinkUrl();
    api_quote_token = client_.getAPI_QuoteToken();
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
        {"acceptAggregationPeriod", FEED_AGGREGATION_PERIOD},
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

auto DX_LinkStreamer::parse_quote(const nlohmann::json& entry) -> MarketQuote
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
    return quote;
}

void DX_LinkStreamer::on_open(connection_hdl hdl)
{
    conn_hdl = std::move(hdl);
    send_initial_msgs();

    keep_alive_thread = std::thread([this]() {
        while (!stop_flag_.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(KEEP_ALIVE_INTERVAL_MS));
            if (!stop_flag_.load())
                send_periodic_msgs();
        }
    });
}

void DX_LinkStreamer::on_msg(const client::message_ptr& msg)
{
    try
    {
        nlohmann::json pckt = nlohmann::json::parse(msg->get_payload());

        if (pckt.contains("type") && pckt["type"] == "FEED_DATA" && pckt.contains("data"))
        {
            for (const auto& entry : pckt["data"])
            {
                // std::cout << "Entry: " << entry << std::endl;
                
                MarketQuote quote = parse_quote(entry);
                if (on_quote) on_quote(quote);
            }
        }
    }
    catch (const std::exception& e)
    {
        LOG_ERROR(std::string("Failed to process message: ") + e.what(), "STREAMER");
    }
}

auto DX_LinkStreamer::create_tls_context() -> std::shared_ptr<boost::asio::ssl::context>
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
        on_open(std::move(hdl));
    });

    c.set_message_handler([this](const connection_hdl&, const client::message_ptr& msg)
    {
        on_msg(msg);
    });

    // tls init handler for secure connections
    c.set_tls_init_handler([this](const connection_hdl&)
    {
        return create_tls_context();
    });
}

auto DX_LinkStreamer::create_connection() -> client::connection_ptr
{
    websocketpp::lib::error_code error_code;
    client::connection_ptr con = c.get_connection(ws_url, error_code);

    if (error_code)
    {
        throw std::runtime_error("Connection initialization error: " + error_code.message());
    }

    return con;
}

void DX_LinkStreamer::run()
{
    running_.store(true);
    configure_channels();
    setup_handlers();

    // create connection
    client::connection_ptr con = create_connection();

    c.connect(con);
    c.run();
}

auto DX_LinkStreamer::safe_parse_quote(const nlohmann::json& pckt, const std::string& key) -> std::optional<double>
{
    if (pckt.contains(key) && pckt[key].is_number()) {
        return pckt[key].get<double>();
    }
    return std::nullopt;
}
