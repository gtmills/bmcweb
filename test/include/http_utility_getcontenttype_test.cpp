// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#include "http_utility.hpp"

#include <gtest/gtest.h>

namespace http_helpers
{
namespace
{

// getContentType is a pure parser – no external dependencies.

TEST(GetContentType, KnownTypes)
{
    EXPECT_EQ(getContentType("application/json"), ContentType::JSON);
    EXPECT_EQ(getContentType("application/cbor"), ContentType::CBOR);
    EXPECT_EQ(getContentType("application/octet-stream"),
              ContentType::OctetStream);
    EXPECT_EQ(getContentType("text/html"), ContentType::HTML);
    EXPECT_EQ(getContentType("text/event-stream"), ContentType::EventStream);
}

TEST(GetContentType, CaseInsensitive)
{
    EXPECT_EQ(getContentType("Application/JSON"), ContentType::JSON);
    EXPECT_EQ(getContentType("APPLICATION/CBOR"), ContentType::CBOR);
    EXPECT_EQ(getContentType("Text/HTML"), ContentType::HTML);
    EXPECT_EQ(getContentType("Text/Event-Stream"), ContentType::EventStream);
}

TEST(GetContentType, WithCharsetParameter)
{
    EXPECT_EQ(getContentType("application/json;charset=UTF-8"),
              ContentType::JSON);
    EXPECT_EQ(getContentType("text/html;charset=utf-8"), ContentType::HTML);
}

TEST(GetContentType, WithSpaceAroundParameter)
{
    // Semicolon followed by a space before the parameter is valid per HTTP.
    EXPECT_EQ(getContentType("application/json; charset=UTF-8"),
              ContentType::JSON);
}

TEST(GetContentType, UnknownType)
{
    EXPECT_EQ(getContentType("text/plain"), ContentType::NoMatch);
    EXPECT_EQ(getContentType("application/xml"), ContentType::NoMatch);
    EXPECT_EQ(getContentType("application/x-www-form-urlencoded"),
              ContentType::NoMatch);
}

TEST(GetContentType, EmptyString)
{
    EXPECT_EQ(getContentType(""), ContentType::NoMatch);
}

TEST(GetContentType, Wildcard)
{
    // "*/*" is not a valid Content-Type (only valid in Accept headers).
    EXPECT_EQ(getContentType("*/*"), ContentType::NoMatch);
}

TEST(GetContentType, Garbage)
{
    EXPECT_EQ(getContentType("not-a-content-type"), ContentType::NoMatch);
    EXPECT_EQ(getContentType("///"), ContentType::NoMatch);
    EXPECT_EQ(getContentType(";"), ContentType::NoMatch);
}

TEST(GetContentType, TrailingGarbage)
{
    // Extra text after a valid type should not match.
    EXPECT_EQ(getContentType("application/json, text/html"),
              ContentType::NoMatch);
    EXPECT_EQ(getContentType("application/json "), ContentType::NoMatch);
}

} // namespace
} // namespace http_helpers
