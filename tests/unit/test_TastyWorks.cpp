#include <gtest/gtest.h>
#include <memory>
#include "../../src/tastyworks/TastyWorks.h"
#include "../../src/log/Log.h"

class test_TastyWorks : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Log::init("src/log/");
    }
};

TEST_F(test_TastyWorks, HandlesTokenGeneration)
{
    std::unique_ptr<TastyWorksClient> twClient = std::make_unique<TastyWorksClient>();
    ASSERT_NE(twClient, nullptr);

    EXPECT_FALSE(twClient->_session_token.empty());
    EXPECT_FALSE(twClient->_api_quote_token.empty());
    EXPECT_FALSE(twClient->_dx_link_url.empty());
}
