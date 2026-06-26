// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#include "async_resp.hpp"
#include "dbus_privileges.hpp"
#include "dbus_utility.hpp"

#include <memory>

#include <gtest/gtest.h>

namespace crow
{
namespace
{

TEST(HandleRequestUserInfo, NoError)
{
    const std::shared_ptr<bmcweb::AsyncResp> asyncResp =
        std::make_shared<bmcweb::AsyncResp>();
    const boost::system::error_code ec = {};
    bool called = false;

    handleRequestUserInfo(
        asyncResp, ec,
        [&called](const dbus::utility::DBusPropertiesMap& /* userInfoMap */) {
            called = true;
        },
        {});
    EXPECT_TRUE(called);
}

TEST(HandleRequestUserInfo, GenericError)
{
    const std::shared_ptr<bmcweb::AsyncResp> asyncResp =
        std::make_shared<bmcweb::AsyncResp>();
    const boost::system::error_code ec = {1, boost::system::system_category()};
    bool called = false;
    handleRequestUserInfo(
        asyncResp, ec,
        [&called](const dbus::utility::DBusPropertiesMap& /* userInfoMap */) {
            called = true;
        },
        {});
    EXPECT_FALSE(called);
    EXPECT_EQ(asyncResp->res.resultInt(), 500);
}

TEST(HandleRequestUserInfo, UserManagerUnreachableError)
{
    const std::shared_ptr<bmcweb::AsyncResp> asyncResp =
        std::make_shared<bmcweb::AsyncResp>();
    const boost::system::error_code ec = {boost::system::errc::host_unreachable,
                                          boost::system::system_category()};

    bool called = false;
    handleRequestUserInfo(
        asyncResp, ec,
        [&called](const dbus::utility::DBusPropertiesMap& /* userInfoMap */) {
            called = true;
        },
        {});
    EXPECT_FALSE(called);
    EXPECT_EQ(asyncResp->res.resultInt(), 500);
}

TEST(HandleRequestUserInfo, UnauthorizedError)
{
    const std::shared_ptr<bmcweb::AsyncResp> asyncResp =
        std::make_shared<bmcweb::AsyncResp>();
    const boost::system::error_code ec = {boost::system::errc::io_error,
                                          boost::system::system_category()};

    bool called = false;
    handleRequestUserInfo(
        asyncResp, ec,
        [&called](const dbus::utility::DBusPropertiesMap& /* userInfoMap */) {
            called = true;
        },
        {});
    EXPECT_FALSE(called);
    EXPECT_EQ(asyncResp->res.resultInt(), 401);
}
// The user properties map forwarded to the callback must contain the exact
// data that was passed to handleRequestUserInfo.
TEST(HandleRequestUserInfo, CallbackReceivesPropertiesMap)
{
    const std::shared_ptr<bmcweb::AsyncResp> asyncResp =
        std::make_shared<bmcweb::AsyncResp>();
    const boost::system::error_code ec = {};

    dbus::utility::DBusPropertiesMap inputMap;
    inputMap.emplace_back("UserPrivilege",
                          dbus::utility::DbusVariantType{std::string("priv-admin")});

    dbus::utility::DBusPropertiesMap capturedMap;
    handleRequestUserInfo(
        asyncResp, ec,
        [&capturedMap](const dbus::utility::DBusPropertiesMap& m) {
            capturedMap = m;
        },
        inputMap);

    ASSERT_EQ(capturedMap.size(), 1);
    EXPECT_EQ(capturedMap[0].first, "UserPrivilege");
    const std::string* val =
        std::get_if<std::string>(&capturedMap[0].second);
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, "priv-admin");
}

// An empty properties map with no error code must still invoke the callback.
TEST(HandleRequestUserInfo, EmptyPropertiesMapCallsCallback)
{
    const std::shared_ptr<bmcweb::AsyncResp> asyncResp =
        std::make_shared<bmcweb::AsyncResp>();
    const boost::system::error_code ec = {};
    bool called = false;

    handleRequestUserInfo(
        asyncResp, ec,
        [&called](const dbus::utility::DBusPropertiesMap&) { called = true; },
        {});

    EXPECT_TRUE(called);
    // Response status must remain at its default (200 OK).
    EXPECT_EQ(asyncResp->res.resultInt(), 200);
}

// A non-io_error, non-host_unreachable error code must produce 500, not 401.
TEST(HandleRequestUserInfo, UnhandledErrorCodeGives500)
{
    const std::shared_ptr<bmcweb::AsyncResp> asyncResp =
        std::make_shared<bmcweb::AsyncResp>();
    // errc::no_such_file_or_directory is a real error but neither io_error
    // nor host_unreachable — it must fall through to the generic 500 path.
    const boost::system::error_code ec = {
        boost::system::errc::no_such_file_or_directory,
        boost::system::system_category()};
    bool called = false;

    handleRequestUserInfo(
        asyncResp, ec,
        [&called](const dbus::utility::DBusPropertiesMap&) { called = true; },
        {});

    EXPECT_FALSE(called);
    EXPECT_EQ(asyncResp->res.resultInt(), 500);
}

} // namespace
} // namespace crow
