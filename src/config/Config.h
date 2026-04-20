#pragma once

#include <string>
#include <nlohmann/json.hpp>

class Config
{
    public:
        static nlohmann::json load_config_file();
        static std::string get_config_value(const std::string& key);
};