#include "Config.h"
#include <fstream>
#include <stdexcept>
#include <filesystem>

auto Config::load_config_file() -> nlohmann::json
{
    namespace fs = std::filesystem;
    
    fs::path currentPath = fs::current_path();
    std::string json_config_fp = (currentPath / "config.json").string();

    if (!fs::exists(json_config_fp))
    {
        throw std::runtime_error("config.json file not found in mochaFiTrader project.");
    }

    std::ifstream file(json_config_fp);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config.json file");
    }

    nlohmann::json config_json;
    file >> config_json;
    file.close();

    return config_json;
}

auto Config::get_config_value(const std::string& key) -> std::string
{
    nlohmann::json config = Config::load_config_file();
    std::string config_value = config.value(key, "");
    return config_value;
}

auto Config::get_numeric_value(const std::string& key) -> double
{
    nlohmann::json config = Config::load_config_file();
    if (!config.contains(key))
        throw std::invalid_argument("Missing required config key: " + key);
    return config.at(key).get<double>();
}