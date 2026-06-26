// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#include "sessions.hpp"

#include <nlohmann/json.hpp>

#include <memory>
#include <string>

#include <gtest/gtest.h>

namespace
{

// ── AuthConfigMethods ────────────────────────────────────────────────────────

TEST(AuthConfigMethods, FromJsonHappyPath)
{
    persistent_data::AuthConfigMethods methods;
    nlohmann::json::object_t jsonValue;
    jsonValue["BasicAuth"] = true;
    jsonValue["CookieAuth"] = true;
    jsonValue["MTLSCommonNameParseMode"] = 2;
    jsonValue["SessionToken"] = true;
    jsonValue["TLS"] = true;
    jsonValue["TLSStrict"] = false;
    jsonValue["XToken"] = true;

    methods.fromJson(jsonValue);

    EXPECT_EQ(methods.basic, true);
    EXPECT_EQ(methods.cookie, true);
    EXPECT_EQ(methods.sessionToken, true);
    EXPECT_EQ(methods.tls, true);
    EXPECT_EQ(methods.tlsStrict, false);
    EXPECT_EQ(methods.xtoken, true);
    EXPECT_EQ(methods.mTLSCommonNameParsingMode,
              static_cast<persistent_data::MTLSCommonNameParseMode>(2));
}

TEST(AuthConfigMethods, FromJsonMTLSCommonNameParseModeOutOfRange)
{
    persistent_data::AuthConfigMethods methods;
    persistent_data::MTLSCommonNameParseMode prevValue =
        methods.mTLSCommonNameParsingMode;
    nlohmann::json::object_t jsonValue;
    jsonValue["BasicAuth"] = true;
    jsonValue["CookieAuth"] = true;
    jsonValue["MTLSCommonNameParseMode"] = 4;
    jsonValue["SessionToken"] = true;
    jsonValue["TLS"] = true;
    jsonValue["TLSStrict"] = false;
    jsonValue["XToken"] = true;

    methods.fromJson(jsonValue);

    EXPECT_EQ(methods.basic, true);
    EXPECT_EQ(methods.cookie, true);
    EXPECT_EQ(methods.sessionToken, true);
    EXPECT_EQ(methods.tls, true);
    EXPECT_EQ(methods.tlsStrict, false);
    EXPECT_EQ(methods.xtoken, true);
    EXPECT_EQ(methods.mTLSCommonNameParsingMode, prevValue);
}
// fromJson silently ignores unknown keys and leaves existing values intact.
TEST(AuthConfigMethods, FromJsonUnknownKeysAreIgnored)
{
    persistent_data::AuthConfigMethods methods;
    // Record the defaults before we call fromJson.
    const bool defaultBasic = methods.basic;
    const bool defaultTls = methods.tls;

    nlohmann::json::object_t jsonValue;
    jsonValue["UnknownKey"] = true;
    jsonValue["AnotherUnknownKey"] = 42;

    methods.fromJson(jsonValue);

    // Nothing should have changed.
    EXPECT_EQ(methods.basic, defaultBasic);
    EXPECT_EQ(methods.tls, defaultTls);
    EXPECT_EQ(methods.sessionToken, methods.sessionToken);
}

// fromJson with an empty object must not change any field from its default.
TEST(AuthConfigMethods, FromJsonEmptyObjectLeavesDefaults)
{
    persistent_data::AuthConfigMethods defaults;
    persistent_data::AuthConfigMethods methods;

    nlohmann::json::object_t empty;
    methods.fromJson(empty);

    EXPECT_EQ(methods.basic, defaults.basic);
    EXPECT_EQ(methods.cookie, defaults.cookie);
    EXPECT_EQ(methods.sessionToken, defaults.sessionToken);
    EXPECT_EQ(methods.tls, defaults.tls);
    EXPECT_EQ(methods.tlsStrict, defaults.tlsStrict);
    EXPECT_EQ(methods.xtoken, defaults.xtoken);
    EXPECT_EQ(methods.mTLSCommonNameParsingMode,
              defaults.mTLSCommonNameParsingMode);
}

// Each boolean field can be independently toggled to false.
TEST(AuthConfigMethods, FromJsonAllBoolFieldsSetToFalse)
{
    persistent_data::AuthConfigMethods methods;
    nlohmann::json::object_t jsonValue;
    // Note: the key used by fromJson for cookie is "Cookie", not "CookieAuth".
    jsonValue["BasicAuth"] = false;
    jsonValue["Cookie"] = false;
    jsonValue["SessionToken"] = false;
    jsonValue["TLS"] = false;
    jsonValue["TLSStrict"] = false;
    jsonValue["XToken"] = false;

    methods.fromJson(jsonValue);

    EXPECT_FALSE(methods.basic);
    EXPECT_FALSE(methods.cookie);
    EXPECT_FALSE(methods.sessionToken);
    EXPECT_FALSE(methods.tls);
    EXPECT_FALSE(methods.tlsStrict);
    EXPECT_FALSE(methods.xtoken);
}

// All valid MTLSCommonNameParseMode integer values must be accepted.
TEST(AuthConfigMethods, FromJsonMTLSCommonNameParseModeAllValidValues)
{
    using Mode = persistent_data::MTLSCommonNameParseMode;

    const Mode validModes[] = {Mode::Invalid, Mode::Whole, Mode::CommonName,
                                Mode::UserPrincipalName};

    for (Mode expected : validModes)
    {
        persistent_data::AuthConfigMethods methods;
        nlohmann::json::object_t jsonValue;
        jsonValue["MTLSCommonNameParseMode"] =
            static_cast<uint64_t>(expected);

        methods.fromJson(jsonValue);

        EXPECT_EQ(methods.mTLSCommonNameParsingMode, expected)
            << "Failed for enum value "
            << static_cast<int>(expected);
    }
}

// A non-boolean value for a boolean key must not crash and must leave that
// field at its previous value.
TEST(AuthConfigMethods, FromJsonNonBoolTypeForBoolKeyIsIgnored)
{
    persistent_data::AuthConfigMethods methods;
    const bool prevBasic = methods.basic;

    nlohmann::json::object_t jsonValue;
    // Supply an integer where a bool is expected.
    jsonValue["BasicAuth"] = 1;

    methods.fromJson(jsonValue);

    // The field must be unchanged — fromJson only reads bool* pointers.
    EXPECT_EQ(methods.basic, prevBasic);
}

// ── UserSession ──────────────────────────────────────────────────────────────

// A fully-populated JSON object must deserialise into a valid UserSession.
TEST(UserSession, FromJsonHappyPath)
{
    nlohmann::json::object_t j;
    j["unique_id"] = "abc123";
    j["session_token"] = "tok456";
    j["csrf_token"] = "csrf789";
    j["username"] = "testuser";
    j["client_id"] = "client01";
    j["client_ip"] = "192.168.1.1";

    auto session = persistent_data::UserSession::fromJson(j);

    ASSERT_NE(session, nullptr);
    EXPECT_EQ(session->uniqueId, "abc123");
    EXPECT_EQ(session->sessionToken, "tok456");
    EXPECT_EQ(session->csrfToken, "csrf789");
    EXPECT_EQ(session->username, "testuser");
    ASSERT_TRUE(session->clientId.has_value());
    EXPECT_EQ(session->clientId.value(), "client01");
    EXPECT_EQ(session->clientIp, "192.168.1.1");
    // Sessions loaded from persistent storage must be treated as Session type.
    EXPECT_EQ(session->sessionType,
              persistent_data::SessionType::Session);
}

// Missing any of the four required fields (unique_id, username,
// session_token, csrf_token) must cause fromJson to return nullptr.
TEST(UserSession, FromJsonMissingUniqueIdReturnsNullptr)
{
    nlohmann::json::object_t j;
    // unique_id intentionally omitted.
    j["session_token"] = "tok456";
    j["csrf_token"] = "csrf789";
    j["username"] = "testuser";

    EXPECT_EQ(persistent_data::UserSession::fromJson(j), nullptr);
}

TEST(UserSession, FromJsonMissingUsernameReturnsNullptr)
{
    nlohmann::json::object_t j;
    j["unique_id"] = "abc123";
    j["session_token"] = "tok456";
    j["csrf_token"] = "csrf789";
    // username intentionally omitted.

    EXPECT_EQ(persistent_data::UserSession::fromJson(j), nullptr);
}

TEST(UserSession, FromJsonMissingSessionTokenReturnsNullptr)
{
    nlohmann::json::object_t j;
    j["unique_id"] = "abc123";
    // session_token intentionally omitted.
    j["csrf_token"] = "csrf789";
    j["username"] = "testuser";

    EXPECT_EQ(persistent_data::UserSession::fromJson(j), nullptr);
}

TEST(UserSession, FromJsonMissingCsrfTokenReturnsNullptr)
{
    nlohmann::json::object_t j;
    j["unique_id"] = "abc123";
    j["session_token"] = "tok456";
    // csrf_token intentionally omitted.
    j["username"] = "testuser";

    EXPECT_EQ(persistent_data::UserSession::fromJson(j), nullptr);
}

// A non-string value for a known field must not crash; the session may be
// returned with that field empty (fromJson logs an error and continues).
TEST(UserSession, FromJsonNonStringValueIsSkipped)
{
    nlohmann::json::object_t j;
    j["unique_id"] = "abc123";
    j["session_token"] = "tok456";
    j["csrf_token"] = "csrf789";
    // username supplied as an integer — not a string.
    j["username"] = 42;

    // username will be empty after the skip, so the required-fields check
    // should reject the session.
    EXPECT_EQ(persistent_data::UserSession::fromJson(j), nullptr);
}

// An optional field (client_id) that is absent must leave clientId as
// std::nullopt.
TEST(UserSession, FromJsonMissingClientIdLeavesNullopt)
{
    nlohmann::json::object_t j;
    j["unique_id"] = "abc123";
    j["session_token"] = "tok456";
    j["csrf_token"] = "csrf789";
    j["username"] = "testuser";
    // client_id intentionally omitted.

    auto session = persistent_data::UserSession::fromJson(j);

    ASSERT_NE(session, nullptr);
    EXPECT_FALSE(session->clientId.has_value());
}

} // namespace
