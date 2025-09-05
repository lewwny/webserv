#include "../include/Parser.hpp"
#include "../include/Response.hpp"
#include "test_harness.hpp"
#include <iostream>
#include <string>

// Simplified integration tests for request->response flow

static void test_complete_get_request_flow()
{
    if (g_verbose) std::cout << "[Test] Complete GET request processing flow" << std::endl;
    
    // 1. Parse raw HTTP request
    Parser parser;
    parser.setLimits(8192, 1024*1024, 4096);
    std::string rawRequest = "GET /index.html HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: Test\r\n\r\n";
    
    if (g_verbose) std::cout << "[Step 1] Parsing raw HTTP request" << std::endl;
    if (g_verbose) std::cout << "[Input]\n" << rawRequest << std::endl;
    
    parser.feed(rawRequest);
    ASSERT_TRUE(parser.isComplete());
    Request request = parser.getRequest();
    
    if (g_verbose) std::cout << "[Result] parsed successfully: method=" << request.getMethod() 
              << ", path=" << request.getPath() 
              << ", host=" << request.getHeader("host") << std::endl;
    
    // 2. Simulate routing logic
    if (g_verbose) std::cout << "[Step 2] Simulating routing logic" << std::endl;
    
    std::string path = request.getPath();
    int status = 200;
    std::string reason = "OK";
    std::string responseBody = "<html><body>Welcome!</body></html>";
    
    if (path == "/index.html") {
        if (g_verbose) std::cout << "[Result] routing to static file" << std::endl;
    } else {
        if (g_verbose) std::cout << "[Result] routing to 404 error" << std::endl;
        status = 404;
        reason = "Not Found";
        responseBody = "<html><body><h1>404 Not Found</h1></body></html>";
    }
    
    // 3. Generate response
    if (g_verbose) std::cout << "[Step 3] Generating response" << std::endl;
    Response response;
    response.setStatus(status, reason);
    response.setHeader("Content-Type", "text/html");
    response.setBody(responseBody);
    
    // 4. Serialize response
    if (g_verbose) std::cout << "[Step 4] Serializing response" << std::endl;
    std::string serializedResponse = response.serialize();
    if (g_verbose) std::cout << "[Result] response ready for transmission (" << serializedResponse.length() << " bytes)" << std::endl;
    
    // Validate complete flow
    ASSERT_TRUE(!serializedResponse.empty());
    ASSERT_TRUE(serializedResponse.find("HTTP/1.1") == 0);
    ASSERT_TRUE(response.getStatusCode() == 200);
}

static void test_complete_post_request_flow()
{
    if (g_verbose) std::cout << "[Test] Complete POST request processing flow" << std::endl;
    
    // 1. Parse POST request with body
    Parser parser;
    parser.setLimits(8192, 1024*1024, 4096);
    std::string rawRequest = "POST /api/data HTTP/1.1\r\n"
                            "Host: localhost:8080\r\n"
                            "Content-Type: application/json\r\n"
                            "Content-Length: 24\r\n"
                            "\r\n"
                            "{\"name\":\"test\",\"id\":123}";
    
    if (g_verbose) std::cout << "[Step 1] Parsing POST request with JSON body" << std::endl;
    if (g_verbose) std::cout << "[Input]\n" << rawRequest << std::endl;
    
    parser.feed(rawRequest);
    ASSERT_TRUE(parser.isComplete());
    Request request = parser.getRequest();
    
    if (g_verbose) std::cout << "[Result] parsed: method=" << request.getMethod() 
              << ", path=" << request.getPath() 
              << ", body_length=" << request.getBody().length() << std::endl;
    
    // 2. Process based on request
    if (g_verbose) std::cout << "[Step 2] Processing POST request" << std::endl;
    Response response;
    
    if (request.getMethod() == "POST" && request.getPath().find("/api/") == 0) {
        if (g_verbose) std::cout << "[Step 3] Processing API POST request" << std::endl;
        response.setStatus(201, "Created");
        response.setHeader("Content-Type", "application/json");
        response.setBody("{\"status\":\"created\",\"id\":123}");
    } else {
        response.setStatus(404, "Not Found");
        response.setBody("Endpoint not found");
    }
    
    std::string serialized = response.serialize();
    if (g_verbose) std::cout << "[Result] POST response generated (" << serialized.length() << " bytes)" << std::endl;
    
    ASSERT_TRUE(!serialized.empty());
    ASSERT_TRUE(response.getStatusCode() == 201 || response.getStatusCode() == 404);
}

static void test_error_handling_flow()
{
    if (g_verbose) std::cout << "[Test] Error handling throughout the flow" << std::endl;
    
    // 1. Malformed request
    Parser parser;
    parser.setLimits(8192, 1024*1024, 4096);
    std::string malformedRequest = "INVALID REQUEST FORMAT\r\n\r\n";
    
    if (g_verbose) std::cout << "[Step 1] Testing malformed request handling" << std::endl;
    if (g_verbose) std::cout << "[Input] " << malformedRequest << std::endl;
    
    parser.feed(malformedRequest);
    Request request = parser.getRequest();
    
    if (g_verbose) std::cout << "[Result] parser error=" << (request.getError() ? "true" : "false");
    if (request.getError()) {
        std::cout << ", code=" << request.getErrorCode();
    }
    std::cout << std::endl;
    
    // 2. Generate error response
    if (request.getError()) {
        if (g_verbose) std::cout << "[Step 2] Generating error response for malformed request" << std::endl;
        Response errorResponse;
        errorResponse.setStatus(request.getErrorCode(), request.getErrorMessage());
        errorResponse.setHeader("Content-Type", "text/html");
        errorResponse.setBody("<html><body><h1>Bad Request</h1></body></html>");
        
        std::string serialized = errorResponse.serialize();
        if (g_verbose) std::cout << "[Result] error response generated: status=" << errorResponse.getStatusCode() << std::endl;
        
        ASSERT_TRUE(errorResponse.getStatusCode() >= 400);
        ASSERT_TRUE(!serialized.empty());
    }
}

static void test_request_with_cookies_flow()
{
    if (g_verbose) std::cout << "[Test] Request with cookies processing flow" << std::endl;
    
    // 1. Parse request with cookies
    Parser parser;
    parser.setLimits(8192, 1024*1024, 4096);
    std::string rawRequest = "GET /profile HTTP/1.1\r\n"
                            "Host: localhost:8080\r\n"
                            "Cookie: sessionId=abc123; authToken=xyz789\r\n"
                            "\r\n";
    
    if (g_verbose) std::cout << "[Step 1] Parsing request with cookies" << std::endl;
    if (g_verbose) std::cout << "[Input]\n" << rawRequest << std::endl;
    
    parser.feed(rawRequest);
    ASSERT_TRUE(parser.isComplete());
    Request request = parser.getRequest();
    
    if (g_verbose) std::cout << "[Result] cookies header: " << request.getHeader("cookie") << std::endl;
    
    // 2. Process request and set response cookies
    if (g_verbose) std::cout << "[Step 2] Processing authenticated request" << std::endl;
    Response response;
    
    std::string cookieHeader = request.getHeader("cookie");
    if (!cookieHeader.empty() && cookieHeader.find("sessionId=") != std::string::npos) {
        if (g_verbose) std::cout << "[Processing] Valid session found" << std::endl;
        response.setStatus(200, "OK");
        response.setHeader("Content-Type", "text/html");
        response.setBody("<html><body>Welcome back!</body></html>");
        // Refresh session cookie
        response.addCookie("sessionId", "abc123", "Path=/; HttpOnly; Max-Age=3600");
    } else {
        if (g_verbose) std::cout << "[Processing] No valid session" << std::endl;
        response.setStatus(401, "Unauthorized");
        response.setBody("Please log in");
    }
    
    std::string serialized = response.serialize();
    if (g_verbose) std::cout << "[Result] response with session handling (" << serialized.length() << " bytes)" << std::endl;
    
    ASSERT_TRUE(!serialized.empty());
    ASSERT_TRUE(response.getStatusCode() == 200 || response.getStatusCode() == 401);
}

static void test_chunked_response_flow()
{
    if (g_verbose) std::cout << "[Test] Chunked response generation flow" << std::endl;
    
    // Simulate large response that should be chunked
    Response response;
    response.setStatus(200, "OK");
    response.setHeader("Content-Type", "text/plain");
    response.setChunked(true);
    
    // Large body content
    std::string largeContent;
    for (int i = 0; i < 100; ++i) {
        largeContent += "This is line " + std::string(1, '0' + (i % 10)) + " of large content.\n";
    }
    response.setBody(largeContent);
    
    if (g_verbose) std::cout << "[Input] Large content (" << largeContent.length() << " bytes)" << std::endl;
    if (g_verbose) std::cout << "[Expect] Chunked encoding response" << std::endl;
    
    std::string serialized = response.serialize();
    if (g_verbose) std::cout << "[Result] chunked=" << (response.isChunked() ? "true" : "false") << std::endl;
    if (g_verbose) std::cout << "[Result] has Transfer-Encoding=" << (serialized.find("Transfer-Encoding: chunked") != std::string::npos ? "true" : "false") << std::endl;
    if (g_verbose) std::cout << "[Result] no Content-Length=" << (serialized.find("Content-Length:") == std::string::npos ? "true" : "false") << std::endl;
    
    ASSERT_TRUE(response.isChunked());
    ASSERT_TRUE(serialized.find("Transfer-Encoding: chunked") != std::string::npos);
    ASSERT_TRUE(serialized.find("Content-Length:") == std::string::npos);
}

int main(int argc, char* argv[])
{
    parse_args(argc, argv);
    
    RUN_TEST(test_complete_get_request_flow);
    RUN_TEST(test_complete_post_request_flow);
    RUN_TEST(test_error_handling_flow);
    RUN_TEST(test_request_with_cookies_flow);
    RUN_TEST(test_chunked_response_flow);
    return finish_tests();
}
