#include "Config.h"
#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <map>

auto Config::load_config_file() -> nlohmann::json
{
    static const nlohmann::json cached = []()
    {
        namespace fs = std::filesystem;

        fs::path currentPath = fs::current_path();
        std::string json_config_fp = (currentPath / "config.json").string();

        if (!fs::exists(json_config_fp))
            throw std::runtime_error("config.json file not found in mochaFiTrader project.");

        std::ifstream file(json_config_fp);
        if (!file.is_open())
            throw std::runtime_error("Could not open config.json file");

        nlohmann::json config_json;
        file >> config_json;
        return config_json;
    }();
    return cached;
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

static auto load_strategy_file(const std::string& strategy) -> const nlohmann::json&
{
    static std::map<std::string, nlohmann::json> cache;

    auto found = cache.find(strategy);
    if (found != cache.end())
        return found->second;

    namespace fs = std::filesystem;
    std::string config_path = (fs::current_path() / "src" / "algo" / "strategy" / (strategy + ".json")).string();

    if (!fs::exists(config_path))
        throw std::runtime_error("Strategy config not found: " + config_path);

    std::ifstream file(config_path);
    if (!file.is_open())
        throw std::runtime_error("Could not open strategy config: " + config_path);

    nlohmann::json config_json;
    file >> config_json;
    cache[strategy] = std::move(config_json);
    return cache.at(strategy);
}

auto Config::get_strategy_value(const std::string& strategy, const std::string& key) -> double
{
    const nlohmann::json& config = load_strategy_file(strategy);
    if (!config.contains(key))
        throw std::invalid_argument("Missing key '" + key + "' in strategy config: " + strategy);
    return config.at(key).get<double>();
}