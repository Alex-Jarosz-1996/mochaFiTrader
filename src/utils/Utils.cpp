#include "Utils.h"

namespace Utils
{

auto isCorrectStatusCode(cpr::Response& resp, int status_code) -> bool
{
    return resp.status_code == status_code;
}

auto parseJsonResponse(cpr::Response& resp) -> nlohmann::json
{
    auto json = nlohmann::json::parse(resp.text);
    if (!json.is_object())
        throw std::invalid_argument("Expected JSON object at root");
    return json;
}

auto doesJsonResponseContainDataAttr(nlohmann::json& json_resp) -> bool
{
    return json_resp.contains("data");
}

auto checkForAttrInsideJsonResponse(nlohmann::json& json_resp, const std::string& msg) -> bool
{
    return json_resp["data"].contains(msg);
}

auto getJsonResponseAttrArraySize(nlohmann::json& json_resp, const std::string& attr) -> size_t
{
    const auto& val = getJsonDataAttr(json_resp, attr);
    return val.is_array() ? val.size() : 0;
}

}
