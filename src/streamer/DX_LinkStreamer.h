#ifndef DX_LINKSTREAMER_H
#define DX_LINKSTREAMER_H

#include <string>
#include <iostream>
#include <nlohmann/json.hpp>
#include "TastyWorks.h"

class DX_LinkStreamer {
private:
    // required for class
    std::string instrument_type;
    std::string instrument;
    TastyWorksClient twClientInstance;

    const int TIMEOUT = 5; // seconds
    const int SETUP_CHANNEL = 0;
    const int FEED_CHANNEL = 0;

    nlohmann::json setup_msg;
    nlohmann::json authorize_msg;
    nlohmann::json channel_request_msg;
    nlohmann::json feed_setup_msg;
    nlohmann::json feed_subscription_msg;
    nlohmann::json keep_alive_msg;

    // from TastyWorksClient class    
    std::string ws_url;
    std::string api_quote_token;
    std::string session_token; 

    void populate_class_attrs();

public:
    DX_LinkStreamer(
        const std::string& instrument_type, 
        const std::string& instrument, 
        TastyWorksClient& twClientInstance
    );

    ~DX_LinkStreamer();

    void setup_messages();
};

#endif // DX_LINKSTREAMER_H