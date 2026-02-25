#pragma once

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <string>

namespace Utils
{

// HTTP helpers
bool isCorrectStatusCode(cpr::Response& r, int sc);
nlohmann::json parseJsonResponse(cpr::Response& r);

// JSON helpers
bool doesJsonResponseContainDataAttr(nlohmann::json& jr);
bool checkForAttrInsideJsonResponse(nlohmann::json& jr, const std::string& msg);

inline const nlohmann::json& getJsonDataAttr(nlohmann::json& jr, const std::string& a)
{
    return jr.at("data").at(a);
}

template <typename T>
T getJsonResponseAttrValue(nlohmann::json& jr, const std::string& a)
{
    return getJsonDataAttr(jr, a).get<T>();
}

size_t getJsonResponseAttrArraySize(nlohmann::json& jr, const std::string& a);

} // namespace Utils
