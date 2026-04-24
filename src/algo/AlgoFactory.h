#pragma once
#include <memory>
#include <string>
#include "src/algo/algo/Algo.h"

namespace AlgoFactory
{
    auto make_strategy(const std::string& name) -> std::unique_ptr<Algo>;
}
