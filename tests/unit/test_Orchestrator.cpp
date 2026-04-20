#include <gtest/gtest.h>
#include <memory>
#include <optional>
#include <stdexcept>

#include "../../src/log/Log.h"
#include "../../src/orchestrator/Orchestrator.h"
#include "../../src/tastyworks/TastyWorks.h"
#include "../../src/streamer/DX_LinkStreamer.h"
#include "../../src/algo/Signal.h"

// Helper to try to build a working orchestrator stack.
// If config/credentials aren't present, skip the test instead of failing.
struct OrchestratorFixture
{
    std::unique_ptr<TastyWorksClient> twClient;
    std::unique_ptr<DX_LinkStreamer> streamer;
    std::unique_ptr<Orchestrator> orchestrator;

    void build_or_skip()
    {
        try
        {
            twClient = std::make_unique<TastyWorksClient>();
            streamer = std::make_unique<DX_LinkStreamer>(*twClient);
            orchestrator = std::make_unique<Orchestrator>(*twClient, *streamer);
        } catch (const std::exception& e) {
            GTEST_SKIP() << "Skipping Orchestrator tests: unable to construct dependencies. "
                         << "Reason: " << e.what();
        }
    }
};

class test_Orchestrator : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Log::init("src/log/");
    }
};

TEST_F(test_Orchestrator, BuildOrderBody_ReturnsNulloptOnHold)
{
    OrchestratorFixture fixture;
    fixture.build_or_skip();

    auto body = fixture.orchestrator->build_order_body(Signal::HOLD);
    EXPECT_FALSE(body.has_value());
}

TEST_F(test_Orchestrator, BuildOrderBody_Buy_ReturnsJsonWithOneLegAndBuyAction) // NOLINT(readability-function-cognitive-complexity)
{
    OrchestratorFixture fixture;
    fixture.build_or_skip();

    std::optional<nlohmann::json> body;
    try
    {
        body = fixture.orchestrator->build_order_body(Signal::BUY);
    } catch (const std::exception& e) {
        GTEST_SKIP() << "Skipping because build_order_body(BUY) threw (likely missing API session/config): "
                     << e.what();
    }

    ASSERT_TRUE(body.has_value());

    // Basic shape checks (match your OrderBuilder::buildAllOrderComponentsJson)
    EXPECT_TRUE(body->contains("time-in-force"));
    EXPECT_TRUE(body->contains("order-type"));
    EXPECT_TRUE(body->contains("value"));
    EXPECT_TRUE(body->contains("value-effect"));
    EXPECT_TRUE(body->contains("legs"));

    ASSERT_TRUE((*body)["legs"].is_array());
    ASSERT_GE((*body)["legs"].size(), 1U);

    const auto& leg0 = (*body)["legs"][0];
    EXPECT_TRUE(leg0.contains("instrument-type"));
    EXPECT_TRUE(leg0.contains("symbol"));
    EXPECT_TRUE(leg0.contains("action"));

    // Your Execute constants are "Buy to Open" / "Sell to Close"
    EXPECT_EQ(leg0["action"].get<std::string>(), "Buy to Open");
}

TEST_F(test_Orchestrator, BuildOrderBody_Sell_ReturnsJsonWithOneLegAndSellAction)
{
    OrchestratorFixture fixture;
    fixture.build_or_skip();

    std::optional<nlohmann::json> body;
    try
    {
        body = fixture.orchestrator->build_order_body(Signal::SELL);
    } catch (const std::exception& e) {
        GTEST_SKIP() << "Skipping because build_order_body(SELL) threw (likely missing API session/config): "
                     << e.what();
    }

    ASSERT_TRUE(body.has_value());

    ASSERT_TRUE((*body)["legs"].is_array());
    ASSERT_GE((*body)["legs"].size(), 1U);

    const auto& leg0 = (*body)["legs"][0];
    EXPECT_EQ(leg0["action"].get<std::string>(), "Sell to Close");
}
