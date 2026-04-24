#include <stdexcept>

#include "OrderBuilder.h"

#include "src/log/Log.h"
#include "src/utils/Utils.h"

OrderBuilder::OrderBuilder() = default;
OrderBuilder::~OrderBuilder() = default;

auto OrderBuilder::timeInForce(std::string value) -> OrderBuilder&
{
    LOG_INFO("Constructing 'time_in_force_' variable.", "ORDER");
    time_in_force_ = std::move(value);
    return *this;
}

auto OrderBuilder::orderType(std::string value) -> OrderBuilder&
{
    LOG_INFO("Constructing 'order_type_' variable.", "ORDER");
    order_type_ = std::move(value);
    return *this;
}

auto OrderBuilder::amount(double value) -> OrderBuilder&
{
    LOG_INFO("Constructing 'amount_' variable.", "ORDER");
    amount_ = value;
    return *this;
}

auto OrderBuilder::transactionType(std::string value) -> OrderBuilder&
{
    LOG_INFO("Constructing 'transaction_type_' variable.", "ORDER");
    transaction_type_ = std::move(value);
    return *this;
}

auto OrderBuilder::addLeg(
    std::string instrument,
    std::string symbol,
    std::string action) -> OrderBuilder&
{
    LOG_INFO("Adding leg", "ORDER");
    Leg leg;
    leg.instrument_type = std::move(instrument);
    leg.symbol = std::move(symbol);
    leg.action = std::move(action);
    legs_.push_back(leg);
    return *this;
}

auto OrderBuilder::dryRun(Mode mode) -> OrderBuilder&
{
    LOG_INFO("Determining if DryRun or Submit.", "ORDER");
    mode_ = mode;
    return *this;
}

auto OrderBuilder::buildAllOrderComponentsJson() const -> nlohmann::json
{
    LOG_INFO("Build Json OrderBuilder.", "ORDER");

    nlohmann::json legs_json = nlohmann::json::array();
    for (const Leg& leg : legs_)
    {
        legs_json.push_back({
            {"instrument-type", leg.instrument_type},
            {"symbol", leg.symbol},
            {"action", leg.action}
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

auto OrderBuilder::buildJsonToSubmitOrder(
    double value,
    const std::string& instrument,
    const std::string& symbol,
    const std::string& action,
    Mode dryRun) -> nlohmann::json
{
    LOG_INFO("Building order submission logic.", "ORDER");

    if (value <= 0.0)
        throw std::invalid_argument("Order amount must be positive, requested value: " + std::to_string(value));

    return this
        ->timeInForce("GTC")
        .orderType("Notional Market")
        .amount(value)
        .transactionType("Debit")
        .addLeg(instrument, symbol, action)
        .dryRun(dryRun)
        .buildAllOrderComponentsJson();
}
