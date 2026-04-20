#pragma once

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <string>

namespace Utils
{

// HTTP helpers
auto isCorrectStatusCode(cpr::Response& resp, int status_code) -> bool;
auto parseJsonResponse(cpr::Response& resp) -> nlohmann::json;

// JSON helpers
auto doesJsonResponseContainDataAttr(nlohmann::json& json_resp) -> bool;
auto checkForAttrInsideJsonResponse(nlohmann::json& json_resp, const std::string& msg) -> bool;

inline auto getJsonDataAttr(nlohmann::json& json_resp, const std::string& attr) -> const nlohmann::json&
{
    return json_resp.at("data").at(attr);
}

template <typename T>
auto getJsonResponseAttrValue(nlohmann::json& json_resp, const std::string& attr) -> T
{
    return getJsonDataAttr(json_resp, attr).get<T>();
}

auto getJsonResponseAttrArraySize(nlohmann::json& json_resp, const std::string& attr) -> size_t;

} // namespace Utils
