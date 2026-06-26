// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#include "filter_expr_executor.hpp"
#include "filter_expr_parser_ast.hpp"
#include "filter_expr_printer.hpp"

#include <nlohmann/json.hpp>

#include <optional>
#include <string_view>

#include <gtest/gtest.h>

namespace redfish
{

static void filterTrue(std::string_view filterExpr, nlohmann::json json)
{
    std::optional<filter_ast::LogicalAnd> ast = parseFilter(filterExpr);
    EXPECT_TRUE(ast);
    if (!ast)
    {
        return;
    }
    EXPECT_EQ(json["Members"].size(), 1);
    EXPECT_TRUE(applyFilterToCollection(json, *ast));
    EXPECT_EQ(json["Members"].size(), 1);
}

static void filterFalse(std::string_view filterExpr, nlohmann::json json)
{
    std::optional<filter_ast::LogicalAnd> ast = parseFilter(filterExpr);
    EXPECT_TRUE(ast);
    if (!ast)
    {
        return;
    }
    EXPECT_EQ(json["Members"].size(), 1);
    EXPECT_TRUE(applyFilterToCollection(json, *ast));
    EXPECT_EQ(json["Members"].size(), 0);
}

TEST(FilterParser, Integers)
{
    const nlohmann::json members = R"({"Members": [{"Count": 2}]})"_json;
    // Forward true conditions
    filterTrue("Count eq 2", members);
    filterTrue("Count ne 3", members);
    filterTrue("Count gt 1", members);
    filterTrue("Count ge 2", members);
    filterTrue("Count lt 3", members);
    filterTrue("Count le 2", members);

    // Reverse true conditions
    filterTrue("2 eq Count", members);
    filterTrue("3 ne Count", members);
    filterTrue("3 gt Count", members);
    filterTrue("2 ge Count", members);
    filterTrue("1 lt Count", members);
    filterTrue("2 le Count", members);

    // Forward false conditions
    filterFalse("Count eq 3", members);
    filterFalse("Count ne 2", members);
    filterFalse("Count gt 2", members);
    filterFalse("Count ge 3", members);
    filterFalse("Count lt 2", members);
    filterFalse("Count le 1", members);

    // Reverse false conditions
    filterFalse("3 eq Count", members);
    filterFalse("2 ne Count", members);
    filterFalse("2 gt Count", members);
    filterFalse("1 ge Count", members);
    filterFalse("2 lt Count", members);
    filterFalse("3 le Count", members);
}

TEST(FilterParser, FloatingPointToInteger)
{
    const nlohmann::json members = R"({"Members": [{"Count": 2.0}]})"_json;
    // Forward true conditions
    filterTrue("Count eq 2", members);
    filterTrue("Count ne 3", members);
    filterTrue("Count gt 1", members);
    filterTrue("Count ge 2", members);
    filterTrue("Count lt 3", members);
    filterTrue("Count le 2", members);

    // Reverse true conditions
    filterTrue("2 eq Count", members);
    filterTrue("3 ne Count", members);
    filterTrue("3 gt Count", members);
    filterTrue("2 ge Count", members);
    filterTrue("1 lt Count", members);
    filterTrue("2 le Count", members);

    // Forward false conditions
    filterFalse("Count eq 3", members);
    filterFalse("Count ne 2", members);
    filterFalse("Count gt 2", members);
    filterFalse("Count ge 3", members);
    filterFalse("Count lt 2", members);
    filterFalse("Count le 1", members);

    // Reverse false conditions
    filterFalse("3 eq Count", members);
    filterFalse("2 ne Count", members);
    filterFalse("2 gt Count", members);
    filterFalse("1 ge Count", members);
    filterFalse("2 lt Count", members);
    filterFalse("3 le Count", members);
}

TEST(FilterParser, FloatingPointToFloatingPoint)
{
    const nlohmann::json members = R"({"Members": [{"Count": 2.0}]})"_json;
    // Forward true conditions
    filterTrue("Count eq 2.0", members);
    filterTrue("Count ne 3.0", members);
    filterTrue("Count gt 1.0", members);
    filterTrue("Count ge 2.0", members);
    filterTrue("Count lt 3.0", members);
    filterTrue("Count le 2.0", members);

    // Reverse true conditions
    filterTrue("2.0 eq Count", members);
    filterTrue("3.0 ne Count", members);
    filterTrue("3.0 gt Count", members);
    filterTrue("2.0 ge Count", members);
    filterTrue("1.0 lt Count", members);
    filterTrue("2.0 le Count", members);

    // Forward false conditions
    filterFalse("Count eq 3.0", members);
    filterFalse("Count ne 2.0", members);
    filterFalse("Count gt 2.0", members);
    filterFalse("Count ge 3.0", members);
    filterFalse("Count lt 2.0", members);
    filterFalse("Count le 1.0", members);

    // Reverse false conditions
    filterFalse("3.0 eq Count", members);
    filterFalse("2.0 ne Count", members);
    filterFalse("2.0 gt Count", members);
    filterFalse("1.0 ge Count", members);
    filterFalse("2.0 lt Count", members);
    filterFalse("3.0 le Count", members);
}

TEST(FilterParser, String)
{
    const nlohmann::json members =
        R"({"Members": [{"SerialNumber": "1234"}]})"_json;
    // Forward true conditions
    filterTrue("SerialNumber eq '1234'", members);
    filterTrue("SerialNumber ne 'NotFoo'", members);
    filterTrue("SerialNumber gt '1233'", members);
    filterTrue("SerialNumber ge '1234'", members);
    filterTrue("SerialNumber lt '1235'", members);
    filterTrue("SerialNumber le '1234'", members);

    // Reverse true conditions
    filterTrue("'1234' eq SerialNumber", members);
    filterTrue("'NotFoo' ne SerialNumber", members);
    filterTrue("'1235' gt SerialNumber", members);
    filterTrue("'1234' ge SerialNumber", members);
    filterTrue("'1233' lt SerialNumber", members);
    filterTrue("'1234' le SerialNumber", members);

    // Forward false conditions
    filterFalse("SerialNumber eq 'NotFoo'", members);
    filterFalse("SerialNumber ne '1234'", members);
    filterFalse("SerialNumber gt '1234'", members);
    filterFalse("SerialNumber ge '1235'", members);
    filterFalse("SerialNumber lt '1234'", members);
    filterFalse("SerialNumber le '1233'", members);

    // Reverse false conditions
    filterFalse("'NotFoo' eq SerialNumber", members);
    filterFalse("'1234' ne SerialNumber", members);
    filterFalse("'1234' gt SerialNumber", members);
    filterFalse("'1233' ge SerialNumber", members);
    filterFalse("'1234' lt SerialNumber", members);
    filterFalse("'1235' le SerialNumber", members);
}

TEST(FilterParser, StringHuman)
{
    // Ensure that we're sorting based on human facing numbers, not
    // lexicographic comparison

    const nlohmann::json members = R"({"Members": [{}]})"_json;
    // Forward true conditions
    filterFalse("'20' eq '3'", members);
    filterTrue("'20' ne '3'", members);
    filterTrue("'20' gt '3'", members);
    filterTrue("'20' ge '3'", members);
    filterFalse("'20' lt '3'", members);
    filterFalse("'20' le '3'", members);
}

TEST(FilterParser, StringSemver)
{
    const nlohmann::json members =
        R"({"Members": [{"Version": "20.0.2"}]})"_json;
    // Forward true conditions
    filterTrue("Version eq '20.0.2'", members);
    filterTrue("Version ne '20.2.0'", members);
    filterTrue("Version gt '20.0.1'", members);
    filterTrue("Version gt '1.9.9'", members);
    filterTrue("Version gt '10.9.9'", members);
}

TEST(FilterParser, Dates)
{
    const nlohmann::json members =
        R"({"Members": [{"Created": "2021-11-30T22:41:35.123+00:00"}]})"_json;

    // Note, all comparisons below differ by a single millisecond
    // Forward true conditions
    filterTrue("Created eq '2021-11-30T22:41:35.123+00:00'", members);
    filterTrue("Created ne '2021-11-30T22:41:35.122+00:00'", members);
    filterTrue("Created gt '2021-11-30T22:41:35.122+00:00'", members);
    filterTrue("Created ge '2021-11-30T22:41:35.123+00:00'", members);
    filterTrue("Created lt '2021-11-30T22:41:35.124+00:00'", members);
    filterTrue("Created le '2021-11-30T22:41:35.123+00:00'", members);

    // Reverse true conditions
    filterTrue("'2021-11-30T22:41:35.123+00:00' eq Created", members);
    filterTrue("'2021-11-30T22:41:35.122+00:00' ne Created", members);
    filterTrue("'2021-11-30T22:41:35.124+00:00' gt Created", members);
    filterTrue("'2021-11-30T22:41:35.123+00:00' ge Created", members);
    filterTrue("'2021-11-30T22:41:35.122+00:00' lt Created", members);
    filterTrue("'2021-11-30T22:41:35.123+00:00' le Created", members);

    // Forward false conditions
    filterFalse("Created eq '2021-11-30T22:41:35.122+00:00'", members);
    filterFalse("Created ne '2021-11-30T22:41:35.123+00:00'", members);
    filterFalse("Created gt '2021-11-30T22:41:35.123+00:00'", members);
    filterFalse("Created ge '2021-11-30T22:41:35.124+00:00'", members);
    filterFalse("Created lt '2021-11-30T22:41:35.123+00:00'", members);
    filterFalse("Created le '2021-11-30T22:41:35.122+00:00'", members);

    // Reverse false conditions
    filterFalse("'2021-11-30T22:41:35.122+00:00' eq Created", members);
    filterFalse("'2021-11-30T22:41:35.123+00:00' ne Created", members);
    filterFalse("'2021-11-30T22:41:35.123+00:00' gt Created", members);
    filterFalse("'2021-11-30T22:41:35.122+00:00' ge Created", members);
    filterFalse("'2021-11-30T22:41:35.123+00:00' lt Created", members);
    filterFalse("'2021-11-30T22:41:35.124+00:00' le Created", members);
}

TEST(FilterParser, NestedProperty)
{
    const nlohmann::json members = R"({"Members": [{"Oem": {
        "OEM": {
          "@odata.type": "#OEMLogEntry.v1_1_0.OEMLogEntry",
          "Key": "Switch_1",
          "ErrorId": "SWITCH_EC_STRAP_MISMATCH"
        }
      }}]})"_json;
    // Forward true conditions
    filterTrue("Oem/OEM/ErrorId eq 'SWITCH_EC_STRAP_MISMATCH'", members);
    filterTrue("Oem/OEM/Key eq 'Switch_1'", members);
    filterFalse("Oem/OEM/ErrorId eq 'EC_STRAP_MISMATCH'", members);
    filterFalse("Oem/OEM/ErrorId eq 'CX7_EC_STRAP_MISMATCH'", members);
    filterFalse("Oem/OEM/ErrorId ne 'SWITCH_EC_STRAP_MISMATCH'", members);
}

// ── Multi-member and combined-logic tests ────────────────────────────────────

// Helper: apply a filter to a multi-member collection and return the
// surviving member count.
static size_t memberCountAfterFilter(std::string_view filterExpr,
                                     nlohmann::json collection)
{
    std::optional<filter_ast::LogicalAnd> ast = parseFilter(filterExpr);
    if (!ast)
    {
        ADD_FAILURE() << "Failed to parse filter: " << filterExpr;
        return 0;
    }
    applyFilterToCollection(collection, *ast);
    return collection["Members"].size();
}

TEST(FilterParser, MultiMemberArrayFilterKeepsMatchingSubset)
{
    // Three members with different Count values; filter should keep only those
    // satisfying the predicate.
    nlohmann::json members =
        R"({"Members": [{"Count": 1}, {"Count": 2}, {"Count": 3}]})"_json;

    EXPECT_EQ(memberCountAfterFilter("Count gt 1", members), 2U);
    EXPECT_EQ(memberCountAfterFilter("Count eq 2", members), 1U);
    EXPECT_EQ(memberCountAfterFilter("Count lt 1", members), 0U);
    EXPECT_EQ(memberCountAfterFilter("Count ge 1", members), 3U);
}

TEST(FilterParser, MemberMissingQueriedPropertyIsFilteredOut)
{
    // A member that does not have the queried key must be excluded, not cause
    // a crash.
    nlohmann::json members =
        R"({"Members": [{"Count": 5}, {"Name": "noProp"}]})"_json;

    // The second member has no "Count" key — it must not pass "Count eq 5".
    EXPECT_EQ(memberCountAfterFilter("Count eq 5", members), 1U);
    // Neither member has "Count eq 99".
    EXPECT_EQ(memberCountAfterFilter("Count eq 99", members), 0U);
}

TEST(FilterParser, AndWithThreeClausesObeysPrecedence)
{
    // All three clauses must be true for a member to survive.
    nlohmann::json members =
        R"({"Members": [{"A": 1, "B": 2, "C": 3},
                        {"A": 1, "B": 9, "C": 3},
                        {"A": 9, "B": 2, "C": 3}]})"_json;

    // Only the first member satisfies all three.
    EXPECT_EQ(
        memberCountAfterFilter("A eq 1 and B eq 2 and C eq 3", members), 1U);
}

TEST(FilterParser, OrWithThreeClausesPassesAny)
{
    nlohmann::json members =
        R"({"Members": [{"Code": 1}, {"Code": 2}, {"Code": 3}]})"_json;

    // Any member with Code 1 or 2 survives; Code 3 should also survive if
    // the third clause matches.
    EXPECT_EQ(memberCountAfterFilter("Code eq 1 or Code eq 2 or Code eq 3",
                                     members),
              3U);
    EXPECT_EQ(memberCountAfterFilter("Code eq 1 or Code eq 2", members), 2U);
}

TEST(FilterParser, CollectionWithNoMembersKeyReturnsFalse)
{
    // applyFilterToCollection on a JSON object without a "Members" key must
    // not crash; it should return false.
    nlohmann::json noMembers = R"({"Name": "empty"})"_json;
    std::optional<filter_ast::LogicalAnd> ast = parseFilter("Name eq 'empty'");
    ASSERT_TRUE(ast);
    EXPECT_FALSE(applyFilterToCollection(noMembers, *ast));
}

} // namespace redfish
