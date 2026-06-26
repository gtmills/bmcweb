// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright OpenBMC Authors
#include "json_html_serializer.hpp"

#include <nlohmann/json.hpp>

#include <string>

#include <gtest/gtest.h>

namespace json_html_util
{
namespace
{

const std::string boilerplateStart =
    "<html>\n"
    "<head>\n"
    "<title>Redfish API</title>\n"
    "<link href=\"/styles/redfish.css\" rel=\"stylesheet\">\n"
    "</head>\n"
    "<body>\n"
    "<div class=\"container\">\n"
    "<img src=\"/images/DMTF_Redfish_logo_2017.svg\" alt=\"redfish\" height=\"406px\" width=\"576px\">\n";

const std::string boilerplateEnd =
    "</div>\n"
    "</body>\n"
    "</html>\n";

TEST(JsonHtmlSerializer, dumpHtmlLink)
{
    std::string out;
    nlohmann::json j;
    j["@odata.id"] = "/redfish/v1";
    dumpHtml(out, j);
    EXPECT_EQ(
        out,
        boilerplateStart +
            "<div class=\"content\">\n"
            "{<div class=tab>&quot@odata.id&quot: <a href=\"/redfish/v1\">\"/redfish/v1\"</a><br></div>}</div>\n" +
            boilerplateEnd);
}

TEST(JsonHtmlSerializer, dumpint)
{
    std::string out;
    nlohmann::json j = 42;
    dumpHtml(out, j);
    EXPECT_EQ(out, boilerplateStart + "<div class=\"content\">\n42</div>\n" +
                       boilerplateEnd);
}

TEST(JsonHtmlSerializer, dumpstring)
{
    std::string out;
    nlohmann::json j = "foobar";
    dumpHtml(out, j);
    EXPECT_EQ(out,
              boilerplateStart + "<div class=\"content\">\n\"foobar\"</div>\n" +
                  boilerplateEnd);
}

TEST(JsonHtmlSerializer, dumpNull)
{
    std::string out;
    nlohmann::json j = nullptr;
    dumpHtml(out, j);
    EXPECT_EQ(out, boilerplateStart + "<div class=\"content\">\nnull</div>\n" +
                       boilerplateEnd);
}

TEST(JsonHtmlSerializer, dumpBoolTrue)
{
    std::string out;
    nlohmann::json j = true;
    dumpHtml(out, j);
    EXPECT_EQ(out, boilerplateStart + "<div class=\"content\">\ntrue</div>\n" +
                       boilerplateEnd);
}

TEST(JsonHtmlSerializer, dumpBoolFalse)
{
    std::string out;
    nlohmann::json j = false;
    dumpHtml(out, j);
    EXPECT_EQ(out, boilerplateStart + "<div class=\"content\">\nfalse</div>\n" +
                       boilerplateEnd);
}

TEST(JsonHtmlSerializer, dumpEmptyObject)
{
    std::string out;
    nlohmann::json j = nlohmann::json::object();
    dumpHtml(out, j);
    EXPECT_EQ(out, boilerplateStart + "<div class=\"content\">\n{}</div>\n" +
                       boilerplateEnd);
}

TEST(JsonHtmlSerializer, dumpEmptyArray)
{
    std::string out;
    nlohmann::json j = nlohmann::json::array();
    dumpHtml(out, j);
    EXPECT_EQ(out, boilerplateStart + "<div class=\"content\">\n[]</div>\n" +
                       boilerplateEnd);
}

TEST(JsonHtmlSerializer, dumpNegativeInt)
{
    std::string out;
    nlohmann::json j = -7;
    dumpHtml(out, j);
    EXPECT_EQ(out, boilerplateStart + "<div class=\"content\">\n-7</div>\n" +
                       boilerplateEnd);
}
} // namespace
} // namespace json_html_util
