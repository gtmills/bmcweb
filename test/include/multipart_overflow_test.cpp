// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#include "multipart_parser.hpp"

#include <string>
#include <string_view>

#include <gtest/gtest.h>

namespace
{

// Helper: build a minimal valid multipart body with a header whose name or
// value is exactly `count` characters long.
static std::string makeBodyWithHeaderName(size_t nameLen)
{
    std::string name(nameLen, 'a');
    return "----XX\r\n" + name +
           ": value\r\n"
           "\r\n"
           "Data\r\n"
           "----XX--\r\n";
}

static std::string makeBodyWithHeaderValue(size_t valueLen)
{
    std::string value(valueLen, 'v');
    return "----XX\r\n"
           "content-type: " +
           value +
           "\r\n"
           "\r\n"
           "Data\r\n"
           "----XX--\r\n";
}

constexpr std::string_view kContentType = "multipart/form-data; boundary=--XX";

// ── ERROR_HEADER_NAME_TOO_LONG ────────────────────────────────────────────────

TEST(MultipartOverflow, HeaderNameAtLimitIsAccepted)
{
    // Exactly 400 characters: should be accepted.
    MultipartParser parser;
    EXPECT_EQ(parser.parse(kContentType, makeBodyWithHeaderName(400)),
              ParserError::PARSER_SUCCESS);
}

TEST(MultipartOverflow, HeaderNameOneBeyondLimitIsRejected)
{
    // 401 characters: must trigger ERROR_HEADER_NAME_TOO_LONG.
    MultipartParser parser;
    EXPECT_EQ(parser.parse(kContentType, makeBodyWithHeaderName(401)),
              ParserError::ERROR_HEADER_NAME_TOO_LONG);
}

TEST(MultipartOverflow, HeaderNameWellBeyondLimitIsRejected)
{
    MultipartParser parser;
    EXPECT_EQ(parser.parse(kContentType, makeBodyWithHeaderName(1000)),
              ParserError::ERROR_HEADER_NAME_TOO_LONG);
}

// ── ERROR_HEADER_VALUE_TOO_LONG ───────────────────────────────────────────────

TEST(MultipartOverflow, HeaderValueAtLimitIsAccepted)
{
    // Exactly 400 characters: should be accepted.
    MultipartParser parser;
    EXPECT_EQ(parser.parse(kContentType, makeBodyWithHeaderValue(400)),
              ParserError::PARSER_SUCCESS);
}

TEST(MultipartOverflow, HeaderValueOneBeyondLimitIsRejected)
{
    // 401 characters: must trigger ERROR_HEADER_VALUE_TOO_LONG.
    MultipartParser parser;
    EXPECT_EQ(parser.parse(kContentType, makeBodyWithHeaderValue(401)),
              ParserError::ERROR_HEADER_VALUE_TOO_LONG);
}

TEST(MultipartOverflow, HeaderValueWellBeyondLimitIsRejected)
{
    MultipartParser parser;
    EXPECT_EQ(parser.parse(kContentType, makeBodyWithHeaderValue(1000)),
              ParserError::ERROR_HEADER_VALUE_TOO_LONG);
}

// ── ERROR_DATA_AFTER_ERROR ────────────────────────────────────────────────────

TEST(MultipartOverflow, ContinuedFeedAfterErrorReturnsDataAfterError)
{
    // Drive the parser into the ERROR state, then feed more data.
    MultipartParser parser;
    ASSERT_EQ(
        parser.start(kContentType),
        ParserError::PARSER_SUCCESS);

    // Bad boundary data drives state to ERROR.
    EXPECT_EQ(parser.parsePart("not-a-valid-boundary-at-all!!!"),
              ParserError::ERROR_BOUNDARY_FORMAT);

    // Any further feed must return ERROR_DATA_AFTER_ERROR.
    EXPECT_EQ(parser.parsePart("more data"), ParserError::ERROR_DATA_AFTER_ERROR);
    EXPECT_EQ(parser.parsePart("even more"), ParserError::ERROR_DATA_AFTER_ERROR);
}

} // namespace
