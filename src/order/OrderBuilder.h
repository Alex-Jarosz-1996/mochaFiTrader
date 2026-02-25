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

    OrderBuilder& timeInForce(std::string v);
    OrderBuilder& orderType(std::string v);
    OrderBuilder& amount(double v);
    OrderBuilder& transactionType(std::string v);
    OrderBuilder& addLeg(
        std::string instrument,
        std::string symbol,
        std::string action
    );
    OrderBuilder& dryRun(Mode m);
    
    nlohmann::json buildAllOrderComponentsJson() const;
    nlohmann::json buildJsonToSubmitOrder(
        double value,
        const std::string& instrument,
        const std::string& symbol,
        const std::string& action,
        Mode dryRun
    );

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
