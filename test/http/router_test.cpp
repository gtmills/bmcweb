// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#include "async_resp.hpp"
#include "http_request.hpp"
#include "routing.hpp"
#include "utility.hpp"

#include <boost/beast/http/verb.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <system_error>

#include <gtest/gtest.h>

// IWYU pragma: no_forward_declare bmcweb::AsyncResp

namespace crow
{
namespace
{

using utility::getParameterTag;

TEST(Router, AllowHeader)
{
    // Callback handler that does nothing
    auto nullCallback =
        [](const Request&, const std::shared_ptr<bmcweb::AsyncResp>&) {};

    Router router;
    std::error_code ec;

    constexpr std::string_view url = "/foo";

    Request req{{boost::beast::http::verb::get, url, 11}, ec};

    // No route should return no methods.
    router.validate();
    EXPECT_EQ(router.findRoute(req).allowHeader, "");
    EXPECT_EQ(router.findRoute(req).route.rule, nullptr);

    router.newRuleTagged<getParameterTag(url)>(std::string(url))
        .methods(boost::beast::http::verb::get)(nullCallback);
    router.validate();
    EXPECT_EQ(router.findRoute(req).allowHeader, "GET");
    EXPECT_NE(router.findRoute(req).route.rule, nullptr);

    Request patchReq{{boost::beast::http::verb::patch, url, 11}, ec};
    EXPECT_EQ(router.findRoute(patchReq).route.rule, nullptr);

    router.newRuleTagged<getParameterTag(url)>(std::string(url))
        .methods(boost::beast::http::verb::patch)(nullCallback);
    router.validate();
    EXPECT_EQ(router.findRoute(req).allowHeader, "GET, PATCH");
    EXPECT_NE(router.findRoute(req).route.rule, nullptr);
    EXPECT_NE(router.findRoute(patchReq).route.rule, nullptr);
}

TEST(Router, OverlapingRoutes)
{
    // Callback handler that does nothing
    auto fooCallback =
        [](const Request&, const std::shared_ptr<bmcweb::AsyncResp>&) {
            EXPECT_FALSE(true);
        };
    bool barCalled = false;
    auto foobarCallback =
        [&barCalled](const Request&, const std::shared_ptr<bmcweb::AsyncResp>&,
                     const std::string& bar) {
            barCalled = true;
            EXPECT_EQ(bar, "bar");
        };

    Router router;
    std::error_code ec;

    router.newRuleTagged<getParameterTag("/foo/<str>")>("/foo/<str>")(
        foobarCallback);
    router.newRuleTagged<getParameterTag("/foo")>("/foo")(fooCallback);
    router.validate();
    {
        constexpr std::string_view url = "/foo/bar";

        auto req = std::make_shared<Request>(
            Request::Body{boost::beast::http::verb::get, url, 11}, ec);

        std::shared_ptr<bmcweb::AsyncResp> asyncResp =
            std::make_shared<bmcweb::AsyncResp>();

        router.handle(req, asyncResp);
    }
    EXPECT_TRUE(barCalled);
}

TEST(Router, 404)
{
    bool notFoundCalled = false;
    // Callback handler that does nothing
    auto nullCallback =
        [&notFoundCalled](const Request&,
                          const std::shared_ptr<bmcweb::AsyncResp>&) {
            notFoundCalled = true;
        };

    Router router;
    std::error_code ec;

    constexpr std::string_view url = "/foo/bar";

    auto req = std::make_shared<Request>(
        Request::Body{boost::beast::http::verb::get, url, 11}, ec);

    router.newRuleTagged<getParameterTag(url)>("/foo/<path>")
        .notFound()(nullCallback);
    router.validate();
    {
        std::shared_ptr<bmcweb::AsyncResp> asyncResp =
            std::make_shared<bmcweb::AsyncResp>();

        router.handle(req, asyncResp);
    }
    EXPECT_TRUE(notFoundCalled);
}

TEST(Router, 405)
{
    // Callback handler that does nothing
    auto nullCallback =
        [](const Request&, const std::shared_ptr<bmcweb::AsyncResp>&) {};
    bool called = false;
    auto notAllowedCallback =
        [&called](const Request&, const std::shared_ptr<bmcweb::AsyncResp>&) {
            called = true;
        };

    Router router;
    std::error_code ec;

    constexpr std::string_view url = "/foo/bar";

    auto req = std::make_shared<Request>(
        Request::Body{boost::beast::http::verb::patch, url, 11}, ec);

    router.newRuleTagged<getParameterTag(url)>(std::string(url))
        .methods(boost::beast::http::verb::get)(nullCallback);
    router.newRuleTagged<getParameterTag(url)>("/foo/<path>")
        .methodNotAllowed()(notAllowedCallback);
    router.validate();
    {
        std::shared_ptr<bmcweb::AsyncResp> asyncResp =
            std::make_shared<bmcweb::AsyncResp>();

        router.handle(req, asyncResp);
    }
    EXPECT_TRUE(called);
}
TEST(Router, MultiParamRoute)
{
    std::string capturedA;
    std::string capturedB;
    auto twoParamCallback =
        [&capturedA,
         &capturedB](const Request&,
                     const std::shared_ptr<bmcweb::AsyncResp>&,
                     const std::string& a, const std::string& b) {
            capturedA = a;
            capturedB = b;
        };

    Router router;
    std::error_code ec;

    constexpr std::string_view pattern = "/chassis/<str>/sensors/<str>";
    router.newRuleTagged<getParameterTag(pattern)>(std::string(pattern))(
        twoParamCallback);
    router.validate();

    constexpr std::string_view url = "/chassis/A/sensors/temp1";
    auto req = std::make_shared<Request>(
        Request::Body{boost::beast::http::verb::get, url, 11}, ec);
    std::shared_ptr<bmcweb::AsyncResp> asyncResp =
        std::make_shared<bmcweb::AsyncResp>();

    router.handle(req, asyncResp);

    EXPECT_EQ(capturedA, "A");
    EXPECT_EQ(capturedB, "temp1");
}

TEST(Router, TrailingSlashDoesNotMatchExactRoute)
{
    bool exactCalled = false;
    auto exactCallback =
        [&exactCalled](const Request&,
                       const std::shared_ptr<bmcweb::AsyncResp>&) {
            exactCalled = true;
        };

    Router router;
    std::error_code ec;

    // Register the route without a trailing slash.
    router.newRuleTagged<getParameterTag("/foo")>("/foo")
        .methods(boost::beast::http::verb::get)(exactCallback);
    router.validate();

    // Request with trailing slash must not match the /foo route.
    constexpr std::string_view urlSlash = "/foo/";
    Request slashReq{
        {boost::beast::http::verb::get, urlSlash, 11}, ec};
    EXPECT_EQ(router.findRoute(slashReq).route.rule, nullptr);
    EXPECT_FALSE(exactCalled);
}

TEST(Router, AllowHeaderIncludesRegisteredMethodsOnly)
{
    auto getCallback =
        [](const Request&, const std::shared_ptr<bmcweb::AsyncResp>&) {};
    auto postCallback =
        [](const Request&, const std::shared_ptr<bmcweb::AsyncResp>&) {};

    Router router;
    std::error_code ec;
    constexpr std::string_view url = "/multi";

    router.newRuleTagged<getParameterTag(url)>(std::string(url))
        .methods(boost::beast::http::verb::get)(getCallback);
    router.newRuleTagged<getParameterTag(url)>(std::string(url))
        .methods(boost::beast::http::verb::post)(postCallback);
    router.validate();

    Request getReq{{boost::beast::http::verb::get, url, 11}, ec};
    // Both GET and POST are registered; the Allow header must list both.
    const std::string allowHdr = router.findRoute(getReq).allowHeader;
    EXPECT_NE(allowHdr.find("GET"), std::string::npos);
    EXPECT_NE(allowHdr.find("POST"), std::string::npos);

    // DELETE is not registered; the rule must not be found.
    Request deleteReq{{boost::beast::http::verb::delete_, url, 11}, ec};
    EXPECT_EQ(router.findRoute(deleteReq).route.rule, nullptr);
}

} // namespace
} // namespace crow
