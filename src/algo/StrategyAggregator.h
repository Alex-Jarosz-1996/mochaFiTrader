#pragma once
#include <memory>
#include <vector>
#include "src/algo/algo/Algo.h"

class StrategyAggregator : public Algo
{
public:
    explicit StrategyAggregator(std::vector<std::unique_ptr<Algo>> strategies);
    ~StrategyAggregator() override = default;

    Signal generate_trading_signal(const MarketQuote& quote) override;

protected:
    void process_quote() override {}

private:
    std::vector<std::unique_ptr<Algo>> strategies_;
};
