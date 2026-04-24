#include "AlgoFactory.h"
#include "src/algo/vmacd/VMACD.h"
#include "src/algo/macd/MACD.h"
#include "src/algo/rsi/RSI.h"
#include <stdexcept>

auto AlgoFactory::make_strategy(const std::string& name) -> std::unique_ptr<Algo>
{
    if (name == "vmacd") return std::make_unique<VMACD>();
    if (name == "macd")  return std::make_unique<MACD>();
    if (name == "rsi")   return std::make_unique<RSI>();
    throw std::invalid_argument("Unknown strategy: " + name);
}

auto AlgoFactory::make_strategies(const std::vector<std::string>& names) -> std::vector<std::unique_ptr<Algo>>
{
    std::vector<std::unique_ptr<Algo>> strategies;
    strategies.reserve(names.size());
    for (const auto& name : names)
        strategies.push_back(make_strategy(name));
    return strategies;
}
