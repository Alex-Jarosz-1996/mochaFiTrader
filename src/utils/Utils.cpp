#include "Utils.h"

namespace Utils
{

bool isCorrectStatusCode(cpr::Response& r, int sc)
{
    return r.status_code == sc;
}

nlohmann::json parseJsonResponse(cpr::Response& r)
{
    return nlohmann::json::parse(r.text);
}

bool doesJsonResponseContainDataAttr(nlohmann::json& jr)
{
    return jr.contains("data");
}

bool checkForAttrInsideJsonResponse(nlohmann::json& jr, const std::string& msg)
{
    return jr["data"].contains(msg);
}

size_t getJsonResponseAttrArraySize(nlohmann::json& jr, const std::string& a)
{
    const auto& v = getJsonDataAttr(jr, a);
    return v.is_array() ? v.size() : 0;
}

}