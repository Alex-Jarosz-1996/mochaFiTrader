#ifndef ORDERBUILDER_H
#define ORDERBUILDER_H

#include <nlohmann/json.hpp>
#include <string>

#include "../../src/algo/Signal.h"

class OrderBuilder {
public:
    OrderBuilder();
    ~OrderBuilder();

    struct Leg {
        std::string instrument_type;
        std::string symbol;
        std::string action;
    };

    enum class Mode { Submit, DryRun };

    auto timeInForce(std::string value) -> OrderBuilder&;
    auto orderType(std::string value) -> OrderBuilder&;
    auto amount(double value) -> OrderBuilder&;
    auto transactionType(std::string value) -> OrderBuilder&;
    auto addLeg(
        std::string instrument,
        std::string symbol,
        std::string action
    ) -> OrderBuilder&;
    auto dryRun(Mode mode) -> OrderBuilder&;

    auto buildAllOrderComponentsJson() const -> nlohmann::json;
    auto buildJsonToSubmitOrder(
        double value,
        const std::string& instrument,
        const std::string& symbol,
        const std::string& action,
        Mode dryRun
    ) -> nlohmann::json;

private:
    // OrderBuilder state
    std::string time_in_force_{"GTC"};
    std::string order_type_{"Notional Market"};
    double amount_{0.0};
    std::string transaction_type_{"Debit"};
    std::vector<Leg> legs_;
    Mode mode_{Mode::DryRun};
};

#endif // ORDERBUILDER_H
