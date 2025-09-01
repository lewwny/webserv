#include "../include/Parser.hpp"
#include "test_harness.hpp"
#include <string>
#include <iostream>

static void test_simple_get()
{
    Parser p;
    p.setLimits(8192, 1024*1024, 4096);
    std::string raw = "GET /hello HTTP/1.1\r\nHost: example.com\r\n\r\n";
    std::cout << "[Test] Simple GET\n";
    std::cout << "[Input]\n" << raw << std::endl;
    std::cout << "[Expect] complete=true, path=/hello, host=example.com\n";
    p.feed(raw);
    Request req = p.getRequest();
    std::cout << "[Result] complete=" << (p.isComplete()?"true":"false")
              << ", error=" << (req.getError()?"true":"false")
              << ", code=" << req.getErrorCode()
              << ", path=" << req.getPath()
              << ", host=" << req.getHeader("host")
              << ", body=" << req.getBody() << std::endl;
    ASSERT_TRUE(p.isComplete());
    ASSERT_EQ(req.getPath(), std::string("/hello"));
    ASSERT_EQ(req.getHeader("host"), std::string("example.com"));
    ASSERT_TRUE(!req.getError());
}

static void test_missing_host_http11()
{
    Parser p;
    p.setLimits(8192, 1024*1024, 4096);
    std::string raw = "GET / HTTP/1.1\r\n\r\n";
    std::cout << "[Test] Missing Host (HTTP/1.1)\n";
    std::cout << "[Input]\n" << raw << std::endl;
    std::cout << "[Expect] error=true, code=400 (Missing Host)\n";
    p.feed(raw);
    Request req = p.getRequest();
    std::cout << "[Result] complete=" << (p.isComplete()?"true":"false")
              << ", error=" << (req.getError()?"true":"false")
              << ", code=" << req.getErrorCode()
              << ", path=" << req.getPath()
              << ", host=" << req.getHeader("host")
              << ", body=" << req.getBody() << std::endl;
    ASSERT_TRUE(req.getError());
    ASSERT_EQ(req.getErrorCode(), 400);
}

static void test_content_length_body()
{
    Parser p;
    p.setLimits(8192, 1024*1024, 4096);
    std::string raw = "POST /submit HTTP/1.1\r\nHost: x\r\nContent-Length: 5\r\n\r\nabcde";
    std::cout << "[Test] POST with Content-Length body\n";
    std::cout << "[Input]\n" << raw << std::endl;
    std::cout << "[Expect] complete=true, body=\"abcde\", content-length=5\n";
    p.feed(raw);
    Request req = p.getRequest();
    std::cout << "[Result] complete=" << (p.isComplete()?"true":"false")
              << ", error=" << (req.getError()?"true":"false")
              << ", code=" << req.getErrorCode()
              << ", path=" << req.getPath()
              << ", host=" << req.getHeader("host")
              << ", body=" << req.getBody() << std::endl;
    ASSERT_TRUE(p.isComplete());
    ASSERT_TRUE(!req.getError());
    ASSERT_EQ(req.getHeader("content-length"), std::string("5"));
    ASSERT_EQ(req.getBody(), std::string("abcde"));
}

static void test_multiple_content_length_error()
{
    Parser p;
    p.setLimits(8192, 1024*1024, 4096);
    std::string raw = "POST / HTTP/1.1\r\nHost: x\r\nContent-Length: 3\r\nContent-Length: 3\r\n\r\nabc";
    std::cout << "[Test] Multiple Content-Length headers (smuggling)\n";
    std::cout << "[Input]\n" << raw << std::endl;
    std::cout << "[Expect] error=true, code=400\n";
    p.feed(raw);
    Request req = p.getRequest();
    std::cout << "[Result] complete=" << (p.isComplete()?"true":"false")
              << ", error=" << (req.getError()?"true":"false")
              << ", code=" << req.getErrorCode()
              << ", path=" << req.getPath()
              << ", host=" << req.getHeader("host")
              << ", body=" << req.getBody() << std::endl;
    ASSERT_TRUE(req.getError());
    ASSERT_EQ(req.getErrorCode(), 400);
}

static void test_partial_headers_split()
{
    Parser p;
    p.setLimits(8192, 1024*1024, 4096);
    std::string part1 = "GET /split HTTP/1.1\r\nHost: ex";
    std::string part2 = "ample.com\r\n\r\n";
    std::cout << "[Test] Partial headers split across feeds\n";
    std::cout << "[Input part1]\n" << part1 << std::endl;
    std::cout << "[Input part2]\n" << part2 << std::endl;
    std::cout << "[Expect] after part1: incomplete; after part2: complete, path=/split, host=example.com\n";
    p.feed(part1);
    Request req = p.getRequest();
    std::cout << "[Result after part1] complete=" << (p.isComplete()?"true":"false")
              << ", error=" << (req.getError()?"true":"false") << std::endl;
    ASSERT_TRUE(!p.isComplete());
    p.feed(part2);
    req = p.getRequest();
    std::cout << "[Result after part2] complete=" << (p.isComplete()?"true":"false")
              << ", error=" << (req.getError()?"true":"false")
              << ", path=" << req.getPath() << ", host=" << req.getHeader("host") << std::endl;
    ASSERT_TRUE(p.isComplete());
    ASSERT_EQ(req.getPath(), std::string("/split"));
    ASSERT_EQ(req.getHeader("host"), std::string("example.com"));
}

static void test_partial_body_split()
{
    Parser p;
    p.setLimits(8192, 1024*1024, 4096);
    std::string hdr = "POST /data HTTP/1.1\r\nHost: x\r\nContent-Length: 10\r\n\r\n";
    std::string body1 = "12345";
    std::string body2 = "67890";
    std::cout << "[Test] Partial body split across feeds\n";
    std::cout << "[Input headers]\n" << hdr << std::endl;
    std::cout << "[Input body part1]\n" << body1 << std::endl;
    std::cout << "[Expect] after headers+part1: incomplete; after part2: complete, body=1234567890\n";
    p.feed(hdr + body1);
    Request req = p.getRequest();
    std::cout << "[Result after part1] complete=" << (p.isComplete()?"true":"false")
              << ", body=" << req.getBody() << std::endl;
    ASSERT_TRUE(!p.isComplete());
    p.feed(body2);
    req = p.getRequest();
    std::cout << "[Result after part2] complete=" << (p.isComplete()?"true":"false")
              << ", body=" << req.getBody() << std::endl;
    ASSERT_TRUE(p.isComplete());
    ASSERT_EQ(req.getBody(), std::string("1234567890"));
}

static void test_non_numeric_content_length()
{
    Parser p;
    p.setLimits(8192, 1024*1024, 4096);
    std::string raw = "POST / HTTP/1.1\r\nHost: x\r\nContent-Length: abc\r\n\r\n";
    std::cout << "[Test] Non-numeric Content-Length\n";
    std::cout << "[Input]\n" << raw << std::endl;
    std::cout << "[Expect] error=true, code=400\n";
    p.feed(raw);
    Request req = p.getRequest();
    std::cout << "[Result] complete=" << (p.isComplete()?"true":"false")
              << ", error=" << (req.getError()?"true":"false")
              << ", code=" << req.getErrorCode() << std::endl;
    ASSERT_TRUE(req.getError());
    ASSERT_EQ(req.getErrorCode(), 400);
}

static void test_content_length_too_large()
{
    Parser p;
    // Set small body limit to trigger 413
    p.setLimits(8192, 5, 4096); // bodyBytes = 5
    std::string raw = "POST /big HTTP/1.1\r\nHost: x\r\nContent-Length: 10\r\n\r\n1234567890";
    std::cout << "[Test] Content-Length exceeds configured body limit\n";
    std::cout << "[Input]\n" << raw << std::endl;
    std::cout << "[Expect] error=true, code=413\n";
    p.feed(raw);
    Request req = p.getRequest();
    std::cout << "[Result] complete=" << (p.isComplete()?"true":"false")
              << ", error=" << (req.getError()?"true":"false")
              << ", code=" << req.getErrorCode() << std::endl;
    ASSERT_TRUE(req.getError());
    ASSERT_EQ(req.getErrorCode(), 413);
}

static void test_invalid_method()
{
    Parser p;
    p.setLimits(8192, 1024*1024, 4096);
    std::string raw = "FOO / HTTP/1.1\r\nHost: x\r\n\r\n";
    std::cout << "[Test] Invalid HTTP method\n";
    std::cout << "[Input]\n" << raw << std::endl;
    std::cout << "[Expect] error=true, code=405\n";
    p.feed(raw);
    Request req = p.getRequest();
    std::cout << "[Result] complete=" << (p.isComplete()?"true":"false")
              << ", error=" << (req.getError()?"true":"false")
              << ", code=" << req.getErrorCode() << std::endl;
    ASSERT_TRUE(req.getError());
    ASSERT_EQ(req.getErrorCode(), 405);
}

int main()
{
    RUN_TEST(test_simple_get);
    RUN_TEST(test_missing_host_http11);
    RUN_TEST(test_content_length_body);
    RUN_TEST(test_multiple_content_length_error);
    RUN_TEST(test_partial_headers_split);
    RUN_TEST(test_partial_body_split);
    RUN_TEST(test_non_numeric_content_length);
    RUN_TEST(test_content_length_too_large);
    RUN_TEST(test_invalid_method);
    return finish_tests();
}
