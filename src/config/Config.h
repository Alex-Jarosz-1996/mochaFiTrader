#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class Config
{
    public:
        static nlohmann::json load_config_file();
        static std::string get_config_value(const std::string& key);
        static std::vector<std::string> get_config_array(const std::string& key);
        static double get_numeric_value(const std::string& key);
        static double get_strategy_value(const std::string& strategy, const std::string& key);
};