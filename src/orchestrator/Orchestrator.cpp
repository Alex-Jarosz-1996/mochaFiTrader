#include "Orchestrator.h"
#include "../../src/order/OrderType.h"
#include "../../src/order/OrderBuilder.h"
#include "../../src/model/TradeableAsset.h"
#include "../../src/log/Log.h"

Orchestrator::Orchestrator(TastyWorksClient& client, DX_LinkStreamer& streamer)
    : twClient_(client),
      dxlStreamer_(streamer),
      trAsset_(TradeableAsset::constructTradeableAsset()) {}

std::optional<nlohmann::json> Orchestrator::build_order_body(Signal signal)
{
    LOG_INFO("Building order body.", "ORCHESTRATOR");
    if (signal == Signal::HOLD) return std::nullopt;
    
    const double balance = twClient_.getTradeableAmount();
    
    std::string_view action;
    if (signal == Signal::BUY) {
        action = Execute::BuyToOpen;
    } else if (signal == Signal::SELL) {
        action = Execute::SellToClose;
    } else {
        return std::nullopt;
    }
    
    OrderBuilder builder;
    nlohmann::json order = builder.buildJsonToSubmitOrder(
        balance,
        trAsset_.instrument,
        trAsset_.symbol,
        std::string(action),
        OrderBuilder::Mode::Submit
    );

    return order;
}

void Orchestrator::on_signal(Signal signal)
{
    LOG_INFO("Executing on signal.", "ORCHESTRATOR");
    const double balance = twClient_.getAccountBalance();
    if (balance <= 0.0)
    {
        throw std::invalid_argument("Unable to trade as account is less than threshold.");
    }
    
    if (signal == Signal::HOLD) return;

    const size_t num_pos = twClient_.getNumberAccountPositions();
    if (signal == Signal::BUY && num_pos > 0)
    {
        // checking for BUY signal and currently have posiions
        // LOGIC: do not want to add to position
        return;
    }

    if (signal == Signal::SELL && num_pos == 0)
    {
        // checking for SELL signal and currently have 0 posiions
        // LOGIC: cannot sell on an empty position
        return;
    }

    std::optional<nlohmann::json> body = build_order_body(signal);
    if (!body) return;

    twClient_.submitOrder(*body);
}