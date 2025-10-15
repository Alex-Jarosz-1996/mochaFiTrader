#include <gtest/gtest.h>
#include <httplib.h>
#include <thread>
#include <iostream>
#include <memory>

#include "TastyWorks.h"
// #include "DX_LinkStreamer.h"
// #include "DB_Client.h"

TEST(TastyWorksClientTest, HandlesTokenGeneration)
{
    std::unique_ptr<TastyWorksClient> twClient = std::make_unique<TastyWorksClient>();
    ASSERT_TRUE(twClient != nullptr);

    ASSERT_TRUE(!twClient->_session_token.empty());
    ASSERT_TRUE(!twClient->_api_quote_token.empty());
    ASSERT_TRUE(!twClient->_dx_link_url.empty());
}

// TEST(DX_LinkStreamerClientTest, HandlesStreamerCreation)
// {
//     std::unique_ptr<DB_Client> db = std::make_unique<DB_Client>();
//     ASSERT_TRUE(db != nullptr);
  
//     std::unique_ptr<TastyWorksClient> twClient = std::make_unique<TastyWorksClient>();
//     ASSERT_TRUE(twClient != nullptr);

//     std::string tradeable_assets = "BTC/USD:CXTALP";
//     std::string instrument_type = "options";
    
//     std::unique_ptr<DX_LinkStreamer> dxlStreamer = std::make_unique<DX_LinkStreamer>(
//         instrument_type, tradeable_assets, *twClient, *db
//     );
//     ASSERT_TRUE(dxlStreamer != nullptr);
// }

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
