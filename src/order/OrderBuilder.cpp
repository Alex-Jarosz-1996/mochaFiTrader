#include "cmath"
#include <stdexcept>

#include "OrderBuilder.h"

#include "../log/Log.h"
#include "../utils/Utils.h"

OrderBuilder::OrderBuilder() {}
OrderBuilder::~OrderBuilder() {}

OrderBuilder& OrderBuilder::timeInForce(std::string v)
{
    LOG_INFO("Constructing 'time_in_force_' variable.", "ORDER");
    time_in_force_ = std::move(v);
    return *this;
}

OrderBuilder& OrderBuilder::orderType(std::string v)
{
    LOG_INFO("Constructing 'order_type_' variable.", "ORDER");
    order_type_ = std::move(v);
    return *this;
}

OrderBuilder& OrderBuilder::amount(double v)
{
    LOG_INFO("Constructing 'amount_' variable.", "ORDER");
    amount_ = v;
    return *this;
}

OrderBuilder& OrderBuilder::transactionType(std::string v)
{
    LOG_INFO("Constructing 'transaction_type_' variable.", "ORDER");
    transaction_type_ = std::move(v);
    return *this;
}

OrderBuilder& OrderBuilder::addLeg(
    std::string instrument,
    std::string symbol,
    std::string action)
{
    LOG_INFO("Adding leg", "ORDER");
    Leg leg;
    leg.instrument_type = std::move(instrument);
    leg.symbol = std::move(symbol);
    leg.action = std::move(action);
    legs_.push_back(leg);
    return *this;
}

OrderBuilder& OrderBuilder::dryRun(Mode m)
{
    LOG_INFO("Determining if DryRun or Submit.", "ORDER");
    mode_ = m;
    return *this;
}

nlohmann::json OrderBuilder::buildAllOrderComponentsJson() const
{
    LOG_INFO("Build Json OrderBuilder.", "ORDER");

    nlohmann::json legs_json = nlohmann::json::array();
    for (const Leg& l : legs_)
    {
        legs_json.push_back({
            {"instrument-type", l.instrument_type},
            {"symbol", l.symbol},
            {"action", l.action}
        });
    }

    return nlohmann::json{
        {"time-in-force", time_in_force_},
        {"order-type", order_type_},
        {"value", amount_},
        {"value-effect", transaction_type_},
        {"legs", legs_json}
    };
}

nlohmann::json OrderBuilder::buildJsonToSubmitOrder(
    double value,
    const std::string& instrument,
    const std::string& symbol,
    const std::string& action,
    Mode dryRun)
{
    LOG_INFO("Building order submission logic.", "ORDER");

    legs_.clear();
    
    return this
        ->timeInForce("GTC")
        .orderType("Notional Market")
        .amount(value)
        .transactionType("Debit")
        .addLeg(instrument, symbol, action)
        .dryRun(dryRun)
        .buildAllOrderComponentsJson();
}
