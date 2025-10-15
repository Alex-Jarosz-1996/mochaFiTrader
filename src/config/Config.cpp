#include "Config.h"
#include <fstream>
#include <stdexcept>
#include <filesystem>

nlohmann::json Config::load_config_file()
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

    nlohmann::json j;
    file >> j;
    file.close();
    
    return j;
}

std::string Config::get_config_value(std::string key)
{
    nlohmann::json config = Config::load_config_file();
    std::string config_value = config.value(key, "");
    return config_value;
}