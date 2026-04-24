#pragma once
#include <memory>
#include <string>
#include <vector>
#include "src/algo/algo/Algo.h"

namespace AlgoFactory
{
    auto make_strategy(const std::string& name) -> std::unique_ptr<Algo>;
    auto make_strategies(const std::vector<std::string>& names) -> std::vector<std::unique_ptr<Algo>>;
}
