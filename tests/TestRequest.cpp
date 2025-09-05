#include "../include/Request.hpp"
#include "test_harness.hpp"
#include <iostream>

static void test_request_basic()
{
    std::cout << "[Test] Request basic setters/getters" << std::endl;
    Request r;
    r.setMethod("GET");
    r.setUri("/hello");
    r.setVersion("HTTP/1.1");
    r.setHeader("Host", "localhost");
    std::cout << "[Expect] method=GET, uri=/hello, host=localhost" << std::endl;
    ASSERT_EQ(r.getMethod(), std::string("GET"));
    ASSERT_EQ(r.getUri(), std::string("/hello"));
    ASSERT_EQ(r.getHeader("Host"), std::string("localhost"));
}

static void test_request_body_append()
{
    std::cout << "[Test] Request appendToBody and getBody" << std::endl;
    Request r;
    r.appendToBody("abc");
    r.appendToBody("def");
    std::cout << "[Expect] body=abcdef" << std::endl;
    ASSERT_EQ(r.getBody(), std::string("abcdef"));
}

int main(int argc, char* argv[])
{
    parse_args(argc, argv);
    
    RUN_TEST(test_request_basic);
    RUN_TEST(test_request_body_append);
    return finish_tests();
}
