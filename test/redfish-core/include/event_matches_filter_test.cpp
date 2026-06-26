// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#include "event_matches_filter.hpp"
#include "event_service_store.hpp"

#include <nlohmann/json.hpp>

#include <string_view>

#include <gtest/gtest.h>

namespace redfish
{

// A default-constructed subscription with no filters must accept every event.
TEST(EventMatchesFilter, DefaultSubscriptionAcceptsAllEvents)
{
    persistent_data::UserSubscription sub;
    nlohmann::json::object_t event;
    EXPECT_TRUE(eventMatchesFilter(sub, event, "Event"));
}

// ── resourceTypes filter ─────────────────────────────────────────────────────

TEST(EventMatchesFilter, ResourceTypesMismatchRejectsEvent)
{
    persistent_data::UserSubscription sub;
    sub.resourceTypes.emplace_back("Task");
    nlohmann::json::object_t event;

    EXPECT_FALSE(eventMatchesFilter(sub, event, "Event"));
}

TEST(EventMatchesFilter, ResourceTypesMatchAcceptsEvent)
{
    persistent_data::UserSubscription sub;
    sub.resourceTypes.emplace_back("Task");
    nlohmann::json::object_t event;

    EXPECT_TRUE(eventMatchesFilter(sub, event, "Task"));
}

// ── registryMsgIds filter ────────────────────────────────────────────────────

TEST(EventMatchesFilter, RegistryMsgIdMatchAcceptsEvent)
{
    persistent_data::UserSubscription sub;
    sub.registryMsgIds.emplace_back("OpenBMC.PostComplete");
    nlohmann::json::object_t event;
    event["MessageId"] = "OpenBMC.0.1.PostComplete";

    EXPECT_TRUE(eventMatchesFilter(sub, event, "Event"));
}

TEST(EventMatchesFilter, RegistryMsgIdDifferentRegistryRejectsEvent)
{
    persistent_data::UserSubscription sub;
    sub.registryMsgIds.emplace_back("OpenBMC.PostComplete");
    nlohmann::json::object_t event;
    event["MessageId"] = "Task.0.1.PostComplete";

    EXPECT_FALSE(eventMatchesFilter(sub, event, "Event"));
}

TEST(EventMatchesFilter, RegistryMsgIdDifferentKeyRejectsEvent)
{
    persistent_data::UserSubscription sub;
    sub.registryMsgIds.emplace_back("OpenBMC.PostComplete");
    nlohmann::json::object_t event;
    event["MessageId"] = "OpenBMC.0.1.NoMatch";

    EXPECT_FALSE(eventMatchesFilter(sub, event, "Event"));
}

TEST(EventMatchesFilter, RegistryMsgIdFilterWithNoMessageIdRejectsEvent)
{
    // When registryMsgIds is set but the event has no MessageId key, the
    // event must be rejected.
    persistent_data::UserSubscription sub;
    sub.registryMsgIds.emplace_back("OpenBMC.PostComplete");
    nlohmann::json::object_t event; // no MessageId

    EXPECT_FALSE(eventMatchesFilter(sub, event, "Event"));
}

// ── registryPrefixes filter ──────────────────────────────────────────────────

TEST(EventMatchesFilter, RegistryPrefixMatchAcceptsEvent)
{
    persistent_data::UserSubscription sub;
    sub.registryPrefixes.emplace_back("OpenBMC");
    nlohmann::json::object_t event;
    event["MessageId"] = "OpenBMC.0.1.PostComplete";

    EXPECT_TRUE(eventMatchesFilter(sub, event, "Event"));
}

TEST(EventMatchesFilter, RegistryPrefixDifferentRegistryRejectsEvent)
{
    persistent_data::UserSubscription sub;
    sub.registryPrefixes.emplace_back("OpenBMC");
    nlohmann::json::object_t event;
    event["MessageId"] = "Task.0.1.PostComplete";

    EXPECT_FALSE(eventMatchesFilter(sub, event, "Event"));
}

// ── originResources filter ───────────────────────────────────────────────────

TEST(EventMatchesFilter, OriginResourceMatchAcceptsEvent)
{
    persistent_data::UserSubscription sub;
    sub.originResources.emplace_back("/redfish/v1/Managers/bmc");
    nlohmann::json::object_t event;
    event["OriginOfCondition"] = "/redfish/v1/Managers/bmc";

    EXPECT_TRUE(eventMatchesFilter(sub, event, "Event"));
}

TEST(EventMatchesFilter, OriginResourceMismatchRejectsEvent)
{
    persistent_data::UserSubscription sub;
    sub.originResources.emplace_back("/redfish/v1/Managers/bmc_not");
    nlohmann::json::object_t event;
    event["OriginOfCondition"] = "/redfish/v1/Managers/bmc";

    EXPECT_FALSE(eventMatchesFilter(sub, event, "Event"));
}

// ── combined filters ─────────────────────────────────────────────────────────

// Both registryPrefix AND originResources must match simultaneously.
TEST(EventMatchesFilter, CombinedRegistryPrefixAndOriginBothMustMatch)
{
    persistent_data::UserSubscription sub;
    sub.registryPrefixes.emplace_back("OpenBMC");
    sub.originResources.emplace_back("/redfish/v1/Managers/bmc");

    // Correct registry + correct origin → accepted.
    {
        nlohmann::json::object_t event;
        event["MessageId"] = "OpenBMC.0.1.PostComplete";
        event["OriginOfCondition"] = "/redfish/v1/Managers/bmc";
        EXPECT_TRUE(eventMatchesFilter(sub, event, "Event"));
    }

    // Wrong registry, correct origin → rejected at registry prefix check.
    {
        nlohmann::json::object_t event;
        event["MessageId"] = "Task.0.1.PostComplete";
        event["OriginOfCondition"] = "/redfish/v1/Managers/bmc";
        EXPECT_FALSE(eventMatchesFilter(sub, event, "Event"));
    }

    // Correct registry, wrong origin → rejected at origin check.
    {
        nlohmann::json::object_t event;
        event["MessageId"] = "OpenBMC.0.1.PostComplete";
        event["OriginOfCondition"] = "/redfish/v1/Managers/other";
        EXPECT_FALSE(eventMatchesFilter(sub, event, "Event"));
    }
}

} // namespace redfish
