// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#include "utils/location_utils.hpp"

#include <optional>
#include <string>

#include "gtest/gtest.h"

namespace redfish::location_util
{
namespace
{

TEST(LocationUtility, ValidLocationType)
{
    EXPECT_EQ(getLocationType("xyz.openbmc_project.Inventory.Connector.Slot"),
              "Slot");
    EXPECT_EQ(
        getLocationType("xyz.openbmc_project.Inventory.Connector.Embedded"),
        "Embedded");
}

TEST(LocationUtility, InvalidLocationType)
{
    EXPECT_EQ(getLocationType(""), std::nullopt);
    EXPECT_EQ(getLocationType("xyz.openbmc_project.Inventory.Connector.RANDOM"),
              std::nullopt);
    EXPECT_EQ(getLocationType("xyz.openbmc_project.Inventory.Connector"),
              std::nullopt);
    EXPECT_EQ(getLocationType("xyz.openbmc_project.Inventory."), std::nullopt);
    EXPECT_EQ(getLocationType("xyz.openbmc_project.Item.Connector"),
              std::nullopt);
    EXPECT_EQ(getLocationType("xyz.openbmc_project.Inventory.Connector.BAY"),
              std::nullopt);
    EXPECT_EQ(getLocationType("xyz.openbmc_project.Inventory.Connector.Bay2"),
              std::nullopt);
}

// Both valid type strings must round-trip: the returned string value must
// exactly match what Redfish expects.
TEST(LocationUtility, SlotReturnsExactString)
{
    auto result =
        getLocationType("xyz.openbmc_project.Inventory.Connector.Slot");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), std::string("Slot"));
}

TEST(LocationUtility, EmbeddedReturnsExactString)
{
    auto result =
        getLocationType("xyz.openbmc_project.Inventory.Connector.Embedded");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), std::string("Embedded"));
}

// A string that shares the valid prefix but has extra characters appended
// must not match (no partial / prefix match).
TEST(LocationUtility, SlotWithSuffixIsRejected)
{
    EXPECT_EQ(
        getLocationType("xyz.openbmc_project.Inventory.Connector.SlotExtra"),
        std::nullopt);
}

TEST(LocationUtility, EmbeddedWithSuffixIsRejected)
{
    EXPECT_EQ(getLocationType(
                  "xyz.openbmc_project.Inventory.Connector.EmbeddedPlus"),
              std::nullopt);
}

// Case sensitivity: type names are case-sensitive, so lower-case variants
// must not match.
TEST(LocationUtility, SlotCaseSensitive)
{
    EXPECT_EQ(getLocationType("xyz.openbmc_project.Inventory.Connector.slot"),
              std::nullopt);
    EXPECT_EQ(getLocationType("xyz.openbmc_project.Inventory.Connector.SLOT"),
              std::nullopt);
}

TEST(LocationUtility, EmbeddedCaseSensitive)
{
    EXPECT_EQ(
        getLocationType("xyz.openbmc_project.Inventory.Connector.embedded"),
        std::nullopt);
    EXPECT_EQ(
        getLocationType("xyz.openbmc_project.Inventory.Connector.EMBEDDED"),
        std::nullopt);
}

} // namespace
} // namespace redfish::location_util
