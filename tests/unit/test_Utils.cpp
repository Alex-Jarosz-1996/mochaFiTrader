#include <gtest/gtest.h>
#include <cpr/response.h>
#include <nlohmann/json.hpp>
#include <string>

#include "../../src/utils/Utils.h"

using nlohmann::json;

static constexpr long HTTP_OK        = 200;
static constexpr long HTTP_NOT_FOUND = 404;

//
// isCorrectStatusCode
//
TEST(test_Utils, IsCorrectStatusCode_ReturnsTrueWhenMatch)
{
    cpr::Response resp;
    resp.status_code = HTTP_OK;
    EXPECT_TRUE(Utils::isCorrectStatusCode(resp, HTTP_OK));
}

TEST(test_Utils, IsCorrectStatusCode_ReturnsFalseWhenMismatch)
{
    cpr::Response resp;
    resp.status_code = HTTP_NOT_FOUND;
    EXPECT_FALSE(Utils::isCorrectStatusCode(resp, HTTP_OK));
}

//
// parseJsonResponse
//
TEST(test_Utils, ParseJsonResponse_ParsesValidJson)
{
    static constexpr int EXPECTED_VALUE = 42;

    cpr::Response resp;
    resp.text = R"({"data":{"value":42}})";

    json parsed = Utils::parseJsonResponse(resp);

    ASSERT_TRUE(parsed.contains("data"));
    EXPECT_EQ(parsed["data"]["value"], EXPECTED_VALUE);
}

TEST(test_Utils, ParseJsonResponse_ThrowsOnInvalidJson)
{
    cpr::Response resp;
    resp.text = "{ invalid json }";

    EXPECT_THROW(Utils::parseJsonResponse(resp), std::exception);
}

//
// doesJsonResponseContainDataAttr
//
TEST(test_Utils, DoesJsonResponseContainDataAttr_ReturnsTrue)
{
    json jobj = { {"data", { {"x", 1} }} };
    EXPECT_TRUE(Utils::doesJsonResponseContainDataAttr(jobj));
}

TEST(test_Utils, DoesJsonResponseContainDataAttr_ReturnsFalse)
{
    json jobj = { {"nope", 1} };
    EXPECT_FALSE(Utils::doesJsonResponseContainDataAttr(jobj));
}

//
// checkForAttrInsideJsonResponse
//
TEST(test_Utils, CheckForAttrInsideJsonResponse_ReturnsTrueWhenPresent)
{
    static constexpr int POSITION_COUNT = 5;
    json jobj = { {"data", { {"positions", POSITION_COUNT} }} };
    EXPECT_TRUE(Utils::checkForAttrInsideJsonResponse(jobj, "positions"));
}

TEST(test_Utils, CheckForAttrInsideJsonResponse_ReturnsFalseWhenMissing)
{
    static constexpr int POSITION_COUNT = 5;
    json jobj = { {"data", { {"positions", POSITION_COUNT} }} };
    EXPECT_FALSE(Utils::checkForAttrInsideJsonResponse(jobj, "orders"));
}

TEST(test_Utils, CheckForAttrInsideJsonResponse_WhenNoDataKey_CurrentImplementationMayThrowOrMutate)
{
    json jobj = { {"x", 1} };

    try {
        bool result = Utils::checkForAttrInsideJsonResponse(jobj, "anything");
        EXPECT_FALSE(result);
    } catch (...) {
        SUCCEED();
    }
}

//
// getJsonDataAttr (inline)
//
TEST(test_Utils, GetJsonDataAttr_ReturnsReferenceToCorrectNode)
{
    static constexpr int ALPHA_VAL    = 123;
    static constexpr int ALPHA_MUTATE = 999;

    json jobj = { {"data", { {"alpha", ALPHA_VAL}, {"beta", "xyz"} }} };

    const json& val1 = Utils::getJsonDataAttr(jobj, "alpha");
    const json& val2 = Utils::getJsonDataAttr(jobj, "beta");

    EXPECT_EQ(val1.get<int>(), ALPHA_VAL);
    EXPECT_EQ(val2.get<std::string>(), "xyz");

    jobj["data"]["alpha"] = ALPHA_MUTATE;
    EXPECT_EQ(val1.get<int>(), ALPHA_MUTATE);
}

TEST(test_Utils, GetJsonDataAttr_ThrowsIfNoDataKey)
{
    json jobj = { {"x", 1} };
    EXPECT_THROW(Utils::getJsonDataAttr(jobj, "alpha"), std::exception);
}

TEST(test_Utils, GetJsonDataAttr_ThrowsIfAttrMissing)
{
    json jobj = { {"data", { {"x", 1} }} };
    EXPECT_THROW(Utils::getJsonDataAttr(jobj, "missing"), std::exception);
}

//
// getJsonResponseAttrValue<T> (template)
//
TEST(test_Utils, GetJsonResponseAttrValue_ReturnsTypedValues)
{
    static constexpr int    INT_VAL    = 42;
    static constexpr double DOUBLE_VAL = 12.5;

    json jobj = {
        {"data", {
            {"b", true},
            {"i", INT_VAL},
            {"d", DOUBLE_VAL},
            {"s", "hello"}
        }}
    };

    EXPECT_EQ(Utils::getJsonResponseAttrValue<bool>(jobj, "b"), true);
    EXPECT_EQ(Utils::getJsonResponseAttrValue<int>(jobj, "i"), INT_VAL);
    EXPECT_DOUBLE_EQ(Utils::getJsonResponseAttrValue<double>(jobj, "d"), DOUBLE_VAL);
    EXPECT_EQ(Utils::getJsonResponseAttrValue<std::string>(jobj, "s"), "hello");
}

TEST(test_Utils, GetJsonResponseAttrValue_ThrowsOnNumericTypeMismatch)
{
    json jobj = { {"data", { {"price", "100.50"} }} };
    EXPECT_THROW((void)Utils::getJsonResponseAttrValue<double>(jobj, "price"), std::exception);
}

TEST(test_Utils, GetJsonResponseAttrValue_ThrowsOnMissingAttr)
{
    json jobj = { {"data", { {"x", 1} }} };
    EXPECT_THROW((void)Utils::getJsonResponseAttrValue<int>(jobj, "missing"), std::exception);
}

TEST(test_Utils, GetJsonResponseAttrValue_ThrowsOnTypeMismatch)
{
    json jobj = { {"data", { {"x", "not_an_int"} }} };
    EXPECT_THROW((void)Utils::getJsonResponseAttrValue<int>(jobj, "x"), std::exception);
}

//
// getJsonResponseAttrArraySize
//
TEST(test_Utils, GetJsonResponseAttrArraySize_ReturnsCorrectSize)
{
    static constexpr size_t ARRAY_SIZE = 4;
    json jobj = { {"data", { {"items", json::array({1, 2, 3, 4})} }} };
    EXPECT_EQ(Utils::getJsonResponseAttrArraySize(jobj, "items"), ARRAY_SIZE);
}

TEST(test_Utils, GetJsonResponseAttrArraySize_ReturnsZeroIfNotArray)
{
    static constexpr int NOT_ARRAY_VAL = 123;
    json jobj = { {"data", { {"items", NOT_ARRAY_VAL} }} };
    EXPECT_EQ(Utils::getJsonResponseAttrArraySize(jobj, "items"), 0U);
}

TEST(test_Utils, GetJsonResponseAttrArraySize_ThrowsIfMissingAttr)
{
    json jobj = { {"data", { {"x", 1} }} };
    EXPECT_THROW((void)Utils::getJsonResponseAttrArraySize(jobj, "items"), std::exception);
}

TEST(test_Utils, GetJsonResponseAttrArraySize_ThrowsIfNoDataKey)
{
    json jobj = { {"x", 1} };
    EXPECT_THROW((void)Utils::getJsonResponseAttrArraySize(jobj, "items"), std::exception);
}

TEST(test_Utils, ParseJsonResponse_HandlesEmptyString)
{
    cpr::Response resp;
    resp.text = "";
    EXPECT_THROW(Utils::parseJsonResponse(resp), std::exception);
}

TEST(test_Utils, CheckForAttrInsideJsonResponse_HandlesNullData)
{
    json jobj = { {"data", nullptr} };
    EXPECT_FALSE(Utils::checkForAttrInsideJsonResponse(jobj, "any_key"));
}

TEST(test_Utils, GetJsonDataAttr_HandlesDeeplyNestedJson)
{
    static constexpr int NESTED_VAL = 100;
    json jobj = { {"data", { {"nested", { {"value", NESTED_VAL} }} }} };
    EXPECT_TRUE(Utils::getJsonDataAttr(jobj, "nested").contains("value"));
}

TEST(test_Utils, GetJsonResponseAttrValue_ThrowsWhenDataNodeIsNotObject)
{
    json jobj = { {"data", "Rate limit exceeded"} };
    EXPECT_THROW((void)Utils::getJsonResponseAttrValue<int>(jobj, "any_key"), std::exception);
}

TEST(test_Utils, ParseJsonResponse_ThrowsOnNonObjectRoot)
{
    cpr::Response resp;
    resp.text = "[1, 2, 3]";
    EXPECT_THROW(Utils::parseJsonResponse(resp), std::exception);
}
