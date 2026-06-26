// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#include "str_utility.hpp"

#include <string>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace
{

using ::testing::ElementsAre;

TEST(Split, PositiveTests)
{
    using bmcweb::split;
    std::vector<std::string> vec;
    split(vec, "xx-abc-xx-abb", '-');
    EXPECT_THAT(vec, ElementsAre("xx", "abc", "xx", "abb"));

    vec.clear();
    split(vec, "", '-');
    EXPECT_THAT(vec, ElementsAre(""));

    vec.clear();
    split(vec, "foo/", '/');
    EXPECT_THAT(vec, ElementsAre("foo", ""));

    vec.clear();
    split(vec, "/bar", '/');
    EXPECT_THAT(vec, ElementsAre("", "bar"));

    vec.clear();
    split(vec, "/", '/');
    EXPECT_THAT(vec, ElementsAre("", ""));
}

TEST(Split, Sensor)
{
    using bmcweb::split;
    std::vector<std::string> vec;
    split(vec, "/xyz/openbmc_project/sensors/unit/name", '/');
    EXPECT_THAT(vec, ElementsAre("", "xyz", "openbmc_project", "sensors",
                                 "unit", "name"));
}

TEST(AsciiToLower, Positive)
{
    using bmcweb::asciiToLower;
    // Noop
    EXPECT_EQ(asciiToLower('a'), 'a');
    EXPECT_EQ(asciiToLower('z'), 'z');
    EXPECT_EQ(asciiToLower('0'), '0');
    EXPECT_EQ(asciiToLower('_'), '_');

    // Converted
    EXPECT_EQ(asciiToLower('A'), 'a');
    EXPECT_EQ(asciiToLower('Z'), 'z');
}

TEST(AsciiToLower, BoundaryChars)
{
    using bmcweb::asciiToLower;
    // Characters just outside A-Z range are not converted
    EXPECT_EQ(asciiToLower('@'), '@'); // one before 'A'
    EXPECT_EQ(asciiToLower('['), '['); // one after 'Z'
    EXPECT_EQ(asciiToLower('`'), '`'); // one before 'a'
    EXPECT_EQ(asciiToLower('{'), '{'); // one after 'z'
    EXPECT_EQ(asciiToLower('5'), '5'); // digit
    EXPECT_EQ(asciiToLower(' '), ' '); // space
}

TEST(AsciiIEquals, Positive)
{
    using bmcweb::asciiIEquals;
    EXPECT_TRUE(asciiIEquals("FOO", "foo"));
    EXPECT_TRUE(asciiIEquals("foo", "foo"));
    EXPECT_TRUE(asciiIEquals("", ""));
    EXPECT_TRUE(asciiIEquals("_", "_"));

    EXPECT_FALSE(asciiIEquals("bar", "foo"));
}

TEST(AsciiIEquals, DifferentLengths)
{
    using bmcweb::asciiIEquals;
    EXPECT_FALSE(asciiIEquals("foo", "fo"));
    EXPECT_FALSE(asciiIEquals("fo", "foo"));
    EXPECT_FALSE(asciiIEquals("", "a"));
    EXPECT_FALSE(asciiIEquals("a", ""));
}

} // namespace
