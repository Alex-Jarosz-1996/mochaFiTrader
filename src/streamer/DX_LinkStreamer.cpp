#include "DX_LinkStreamer.h"

DX_LinkStreamer::DX_LinkStreamer(
    const std::string& instrument_type, 
    const std::string& instrument, 
    TastyWorksClient& twClientInstance
) : instrument_type(instrument_type), instrument(instrument), twClientInstance(twClientInstance)
{
    populate_class_attrs();
    setup_messages();
}

DX_LinkStreamer::~DX_LinkStreamer() {}

void DX_LinkStreamer::populate_class_attrs() {
    ws_url = twClientInstance._dx_link_url;
    api_quote_token = twClientInstance._api_quote_token;
    session_token = twClientInstance._session_token;
}

void DX_LinkStreamer::setup_messages() {
    setup_msg = nlohmann::json{
        {"type", "SETUP"},
        {"channel", SETUP_CHANNEL},
        {"version", "0.1-DXF-JS/0.3.0"},
        {"keepaliveTimeout", TIMEOUT},
        {"acceptKeepaliveTimeout", TIMEOUT}
    };

    authorize_msg = nlohmann::json{
        {"type", "AUTH"},
        {"channel", SETUP_CHANNEL},
        {"token", session_token}
    };

    channel_request_msg = nlohmann::json{
        {"type", "CHANNEL_REQUEST"},
        {"channel", FEED_CHANNEL},
        {"service", "FEED"},
        {"parameters", {{"contract", "AUTO"}}}
    };

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

    keep_alive_msg = nlohmann::json{
        {"type", "KEEPALIVE"},
        {"channel", FEED_CHANNEL}
    };
}
