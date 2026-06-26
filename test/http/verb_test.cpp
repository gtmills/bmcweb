// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#include "verb.hpp"

#include <boost/beast/http/verb.hpp>

#include <map>
#include <optional>
#include <string_view>

#include <gtest/gtest.h>

using BoostVerb = boost::beast::http::verb;

TEST(BoostToHttpVerb, ValidCase)
{
    std::map<HttpVerb, BoostVerb> verbMap = {
        {HttpVerb::Delete, BoostVerb::delete_},
        {HttpVerb::Get, BoostVerb::get},
        {HttpVerb::Head, BoostVerb::head},
        {HttpVerb::Options, BoostVerb::options},
        {HttpVerb::Patch, BoostVerb::patch},
        {HttpVerb::Post, BoostVerb::post},
        {HttpVerb::Put, BoostVerb::put},
    };

    for (int verbIndex = 0; verbIndex < static_cast<int>(HttpVerb::Max);
         ++verbIndex)
    {
        HttpVerb httpVerb = static_cast<HttpVerb>(verbIndex);
        std::optional<HttpVerb> verb = httpVerbFromBoost(verbMap[httpVerb]);
        EXPECT_EQ(verb, httpVerb);
    }
}

TEST(BoostToHttpVerbTest, InvalidCase)
{
    std::optional<HttpVerb> verb = httpVerbFromBoost(BoostVerb::unknown);
    EXPECT_FALSE(verb.has_value());
}

TEST(HttpVerbToStringTest, ValidCase)
{
    std::map<HttpVerb, std::string_view> verbMap = {
        {HttpVerb::Delete, "DELETE"}, {HttpVerb::Get, "GET"},
        {HttpVerb::Head, "HEAD"},     {HttpVerb::Options, "OPTIONS"},
        {HttpVerb::Patch, "PATCH"},   {HttpVerb::Post, "POST"},
        {HttpVerb::Put, "PUT"},
    };

    for (int verbIndex = 0; verbIndex < static_cast<int>(HttpVerb::Max);
         ++verbIndex)
    {
        HttpVerb httpVerb = static_cast<HttpVerb>(verbIndex);
        EXPECT_EQ(httpVerbToString(httpVerb), verbMap[httpVerb]);
    }
}

TEST(HttpVerbToStringTest, MaxReturnsEmpty)
{
    // HttpVerb::Max is the sentinel value and has no string representation.
    EXPECT_EQ(httpVerbToString(HttpVerb::Max), "");
}

TEST(BoostToHttpVerb, RoundTrip)
{
    // Every valid HttpVerb should survive a round-trip through boost and back.
    for (int verbIndex = 0; verbIndex < static_cast<int>(HttpVerb::Max);
         ++verbIndex)
    {
        HttpVerb original = static_cast<HttpVerb>(verbIndex);
        std::string_view name = httpVerbToString(original);
        boost::beast::http::verb bv =
            boost::beast::http::string_to_verb(name);
        std::optional<HttpVerb> result = httpVerbFromBoost(bv);
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(*result, original);
    }
}
