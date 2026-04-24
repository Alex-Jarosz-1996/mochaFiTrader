#include "StrategyAggregator.h"
#include <stdexcept>

StrategyAggregator::StrategyAggregator(std::vector<std::unique_ptr<Algo>> strategies)
    : strategies_(std::move(strategies))
{
    if (strategies_.empty())
        throw std::invalid_argument("StrategyAggregator requires at least one strategy.");
}

auto StrategyAggregator::generate_trading_signal(const MarketQuote& quote) -> Signal
{
    Signal consensus = strategies_[0]->generate_trading_signal(quote);
    for (std::size_t i = 1; i < strategies_.size(); ++i)
    {
        if (strategies_[i]->generate_trading_signal(quote) != consensus)
            return Signal::HOLD;
    }
    return consensus;
}
