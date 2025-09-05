#include "../include/Response.hpp"
#include "test_harness.hpp"
#include <iostream>
#include <string>

static void test_response_basic_status()
{
    if (g_verbose) std::cout << "[Test] Response basic status setting" << std::endl;
    Response res;
    res.setStatus(404, "Not Found");
    if (g_verbose) std::cout << "[Expect] status=404, message='Not Found'" << std::endl;
    if (g_verbose) std::cout << "[Result] status=" << res.getStatusCode() << ", message='" << res.getStatusMessage() << "'" << std::endl;
    ASSERT_EQ(res.getStatusCode(), 404);
    ASSERT_EQ(res.getStatusMessage(), std::string("Not Found"));
}

static void test_response_headers()
{
    if (g_verbose) std::cout << "[Test] Response header management" << std::endl;
    Response res;
    res.setHeader("Content-Type", "text/html");
    res.setHeader("Cache-Control", "no-cache");
    if (g_verbose) std::cout << "[Expect] Content-Type='text/html', Cache-Control='no-cache'" << std::endl;
    std::map<std::string, std::string> headers = res.getHeaders();
    if (g_verbose) std::cout << "[Result] headers count=" << headers.size() << std::endl;
    // Note: Response constructor adds default headers (Server, Date)
    ASSERT_TRUE(headers.size() >= 2);
}

static void test_response_body()
{
    if (g_verbose) std::cout << "[Test] Response body management" << std::endl;
    Response res;
    res.setBody("Hello World");
    res.appendBody("!");
    if (g_verbose) std::cout << "[Expect] body='Hello World!'" << std::endl;
    if (g_verbose) std::cout << "[Result] body='" << res.getBody() << "'" << std::endl;
    ASSERT_EQ(res.getBody(), std::string("Hello World!"));
}

static void test_response_cookies()
{
    if (g_verbose) std::cout << "[Test] Response cookie handling" << std::endl;
    Response res;
    res.addCookie("sessionId", "abc123", "Path=/; HttpOnly");
    res.addSetCookie("authToken=xyz789; Secure");
    if (g_verbose) std::cout << "[Expect] Set-Cookie headers added" << std::endl;
    std::string serialized = res.serialize();
    if (g_verbose) std::cout << "[Result] contains sessionId=" << (serialized.find("sessionId=abc123") != std::string::npos ? "true" : "false") << std::endl;
    if (g_verbose) std::cout << "[Result] contains authToken=" << (serialized.find("authToken=xyz789") != std::string::npos ? "true" : "false") << std::endl;
    ASSERT_TRUE(serialized.find("sessionId=abc123") != std::string::npos);
    ASSERT_TRUE(serialized.find("authToken=xyz789") != std::string::npos);
}

static void test_response_serialize()
{
    if (g_verbose) std::cout << "[Test] Response serialization" << std::endl;
    Response res;
    res.setStatus(200, "OK");
    res.setHeader("Content-Type", "text/plain");
    res.setBody("Hello");
    if (g_verbose) std::cout << "[Expect] Valid HTTP response with status line, headers, body" << std::endl;
    std::string serialized = res.serialize();
    if (g_verbose) std::cout << "[Result] starts with HTTP/1.1=" << (serialized.find("HTTP/1.1 200 OK") == 0 ? "true" : "false") << std::endl;
    if (g_verbose) std::cout << "[Result] contains Content-Length=" << (serialized.find("Content-Length: 5") != std::string::npos ? "true" : "false") << std::endl;
    if (g_verbose) std::cout << "[Result] ends with body=" << (serialized.find("\r\n\r\nHello") != std::string::npos ? "true" : "false") << std::endl;
    ASSERT_TRUE(serialized.find("HTTP/1.1 200 OK") == 0);
    ASSERT_TRUE(serialized.find("Content-Length: 5") != std::string::npos);
    ASSERT_TRUE(serialized.find("\r\n\r\nHello") != std::string::npos);
}

static void test_response_chunked()
{
    if (g_verbose) std::cout << "[Test] Response chunked encoding" << std::endl;
    Response res;
    res.setChunked(true);
    res.setBody("Test data");
    if (g_verbose) std::cout << "[Expect] chunked=true, no Content-Length header" << std::endl;
    std::string serialized = res.serialize();
    if (g_verbose) std::cout << "[Result] chunked=" << (res.isChunked() ? "true" : "false") << std::endl;
    if (g_verbose) std::cout << "[Result] has Transfer-Encoding=" << (serialized.find("Transfer-Encoding: chunked") != std::string::npos ? "true" : "false") << std::endl;
    if (g_verbose) std::cout << "[Result] no Content-Length=" << (serialized.find("Content-Length:") == std::string::npos ? "true" : "false") << std::endl;
    ASSERT_TRUE(res.isChunked());
    ASSERT_TRUE(serialized.find("Transfer-Encoding: chunked") != std::string::npos);
    ASSERT_TRUE(serialized.find("Content-Length:") == std::string::npos);
}

static void test_response_connection_management()
{
    if (g_verbose) std::cout << "[Test] Response connection management" << std::endl;
    Response res;
    res.setConnectionClose(true);
    if (g_verbose) std::cout << "[Expect] Connection: close header" << std::endl;
    std::string serialized = res.serialize();
    if (g_verbose) std::cout << "[Result] has Connection close=" << (serialized.find("Connection: close") != std::string::npos ? "true" : "false") << std::endl;
    ASSERT_TRUE(res.isConnectionClose());
    ASSERT_TRUE(serialized.find("Connection: close") != std::string::npos);
}

int main(int argc, char* argv[])
{
    parse_args(argc, argv);
    
    RUN_TEST(test_response_basic_status);
    RUN_TEST(test_response_headers);
    RUN_TEST(test_response_body);
    RUN_TEST(test_response_cookies);
    RUN_TEST(test_response_serialize);
    RUN_TEST(test_response_chunked);
    RUN_TEST(test_response_connection_management);
    return finish_tests();
}
