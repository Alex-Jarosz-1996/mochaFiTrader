#include <gtest/gtest.h>
#include <cpr/response.h>
#include <nlohmann/json.hpp>
#include <string>

#include "../../src/utils/Utils.h"

using nlohmann::json;

//
// isCorrectStatusCode
//
TEST(test_Utils, IsCorrectStatusCode_ReturnsTrueWhenMatch)
{
    cpr::Response r;
    r.status_code = 200;
    EXPECT_TRUE(Utils::isCorrectStatusCode(r, 200));
}

TEST(test_Utils, IsCorrectStatusCode_ReturnsFalseWhenMismatch)
{
    cpr::Response r;
    r.status_code = 404;
    EXPECT_FALSE(Utils::isCorrectStatusCode(r, 200));
}

//
// parseJsonResponse
//
TEST(test_Utils, ParseJsonResponse_ParsesValidJson)
{
    cpr::Response r;
    r.text = R"({"data":{"value":42}})";

    json j = Utils::parseJsonResponse(r);

    ASSERT_TRUE(j.contains("data"));
    EXPECT_EQ(j["data"]["value"], 42);
}

TEST(test_Utils, ParseJsonResponse_ThrowsOnInvalidJson)
{
    cpr::Response r;
    r.text = "{ invalid json }";

    EXPECT_THROW(Utils::parseJsonResponse(r), std::exception);
}

//
// doesJsonResponseContainDataAttr
//
TEST(test_Utils, DoesJsonResponseContainDataAttr_ReturnsTrue)
{
    json j = { {"data", { {"x", 1} }} };
    EXPECT_TRUE(Utils::doesJsonResponseContainDataAttr(j));
}

TEST(test_Utils, DoesJsonResponseContainDataAttr_ReturnsFalse)
{
    json j = { {"nope", 1} };
    EXPECT_FALSE(Utils::doesJsonResponseContainDataAttr(j));
}

//
// checkForAttrInsideJsonResponse
//
TEST(test_Utils, CheckForAttrInsideJsonResponse_ReturnsTrueWhenPresent)
{
    json j = { {"data", { {"positions", 5} }} };
    EXPECT_TRUE(Utils::checkForAttrInsideJsonResponse(j, "positions"));
}

TEST(test_Utils, CheckForAttrInsideJsonResponse_ReturnsFalseWhenMissing)
{
    json j = { {"data", { {"positions", 5} }} };
    EXPECT_FALSE(Utils::checkForAttrInsideJsonResponse(j, "orders"));
}

TEST(test_Utils, CheckForAttrInsideJsonResponse_WhenNoDataKey_CurrentImplementationMayThrowOrMutate)
{
    // NOTE:
    // Your implementation uses jr["data"].contains(msg)
    // operator[] can insert "data" if it doesn't exist.
    //
    // This test documents the current behavior and protects you from surprises.
    json j = { {"x", 1} };

    // Depending on json library behavior, this may:
    //  - create "data": null, then contains() false
    //  - or throw if contains() called on non-object
    //
    // We'll accept either "false" or "throw" as current behavior.
    try {
        bool r = Utils::checkForAttrInsideJsonResponse(j, "anything");
        EXPECT_FALSE(r);
    } catch (...) {
        SUCCEED();
    }
}

//
// getJsonDataAttr (inline)
//
TEST(test_Utils, GetJsonDataAttr_ReturnsReferenceToCorrectNode)
{
    json j = { {"data", { {"alpha", 123}, {"beta", "xyz"} }} };

    const json& v1 = Utils::getJsonDataAttr(j, "alpha");
    const json& v2 = Utils::getJsonDataAttr(j, "beta");

    EXPECT_EQ(v1.get<int>(), 123);
    EXPECT_EQ(v2.get<std::string>(), "xyz");

    // Also show it's a reference: if we modify underlying json, reference reflects it.
    j["data"]["alpha"] = 999;
    EXPECT_EQ(v1.get<int>(), 999);
}

TEST(test_Utils, GetJsonDataAttr_ThrowsIfNoDataKey)
{
    json j = { {"x", 1} };
    EXPECT_THROW(Utils::getJsonDataAttr(j, "alpha"), std::exception);
}

TEST(test_Utils, GetJsonDataAttr_ThrowsIfAttrMissing)
{
    json j = { {"data", { {"x", 1} }} };
    EXPECT_THROW(Utils::getJsonDataAttr(j, "missing"), std::exception);
}

//
// getJsonResponseAttrValue<T> (template)
//
TEST(test_Utils, GetJsonResponseAttrValue_ReturnsTypedValues)
{
    json j = {
        {"data", {
            {"b", true},
            {"i", 42},
            {"d", 12.5},
            {"s", "hello"}
        }}
    };

    EXPECT_EQ(Utils::getJsonResponseAttrValue<bool>(j, "b"), true);
    EXPECT_EQ(Utils::getJsonResponseAttrValue<int>(j, "i"), 42);
    EXPECT_DOUBLE_EQ(Utils::getJsonResponseAttrValue<double>(j, "d"), 12.5);
    EXPECT_EQ(Utils::getJsonResponseAttrValue<std::string>(j, "s"), "hello");
}

TEST(test_Utils, GetJsonResponseAttrValue_ThrowsOnMissingAttr)
{
    json j = { {"data", { {"x", 1} }} };
    EXPECT_THROW((void)Utils::getJsonResponseAttrValue<int>(j, "missing"), std::exception);
}

TEST(test_Utils, GetJsonResponseAttrValue_ThrowsOnTypeMismatch)
{
    json j = { {"data", { {"x", "not_an_int"} }} };

    // nlohmann::json throws type_error on invalid conversions
    EXPECT_THROW((void)Utils::getJsonResponseAttrValue<int>(j, "x"), std::exception);
}

//
// getJsonResponseAttrArraySize
//
TEST(test_Utils, GetJsonResponseAttrArraySize_ReturnsCorrectSize)
{
    json j = { {"data", { {"items", json::array({1,2,3,4})} }} };
    EXPECT_EQ(Utils::getJsonResponseAttrArraySize(j, "items"), 4u);
}

TEST(test_Utils, GetJsonResponseAttrArraySize_ReturnsZeroIfNotArray)
{
    json j = { {"data", { {"items", 123} }} };
    EXPECT_EQ(Utils::getJsonResponseAttrArraySize(j, "items"), 0u);
}

TEST(test_Utils, GetJsonResponseAttrArraySize_ThrowsIfMissingAttr)
{
    json j = { {"data", { {"x", 1} }} };
    EXPECT_THROW((void)Utils::getJsonResponseAttrArraySize(j, "items"), std::exception);
}

TEST(test_Utils, GetJsonResponseAttrArraySize_ThrowsIfNoDataKey)
{
    json j = { {"x", 1} };
    EXPECT_THROW((void)Utils::getJsonResponseAttrArraySize(j, "items"), std::exception);
}