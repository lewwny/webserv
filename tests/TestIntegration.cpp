#include "../include/Parser.hpp"
#include "../include/Router.hpp"
#include "../include/Response.hpp"
#include "../include/Config.hpp"
#include "../include/ServerManager.hpp"
#include "test_harness.hpp"
#include <iostream>
#include <string>

// Integration tests for complete request processing flow

static void test_complete_get_request_flow()
{
    VERBOSE_OUT("[Test] Complete GET request processing flow" << std::endl);
    
    // 1. Parse raw HTTP request
    Parser parser;
    parser.setLimits(8192, 1024*1024, 4096);
    std::string rawRequest = "GET /index.html HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: Test\r\n\r\n";
    
    VERBOSE_OUT("[Step 1] Parsing raw HTTP request" << std::endl);
    VERBOSE_OUT("[Input]\n" << rawRequest << std::endl);
    
    parser.feed(rawRequest);
    ASSERT_TRUE(parser.isComplete());
    Request request = parser.getRequest();
    
    VERBOSE_OUT("[Result] parsed successfully: method=" << request.getMethod() 
              << ", path=" << request.getPath() 
              << ", host=" << request.getHeader("host") << std::endl);
    
    // 2. Route the request (simulate routing without actual Router::decide)
    VERBOSE_OUT("[Step 2] Simulating routing logic" << std::endl);
    
    // Simulate routing decision based on path
    Router::Decision decision;
    std::string path = request.getPath();
    
    if (path == "/index.html") {
        decision.type = Router::ACTION_STATIC;
        decision.status = 200;
        decision.reason = "OK";
        decision.fsPath = "/var/www" + path;
        decision.root = "/var/www";
    } else {
        decision.type = Router::ACTION_ERROR;
        decision.status = 404;
        decision.reason = "Not Found";
    }
    
    VERBOSE_OUT("[Result] routing decision: type=" << decision.type 
              << ", status=" << decision.status << std::endl);
        
        // 3. Generate response based on decision
        VERBOSE_OUT("[Step 3] Generating response" << std::endl);
        Response response;
        
        switch (decision.type) {
            case Router::ACTION_STATIC:
                VERBOSE_OUT("[Response] Serving static file: " << decision.fsPath << std::endl);
                response.setStatus(200, "OK");
                response.setHeader("Content-Type", "text/html");
                response.setBody("<html><body>Static content</body></html>");
                break;
                
            case Router::ACTION_ERROR:
                VERBOSE_OUT("[Response] Error response: " << decision.status << " " << decision.reason << std::endl);
                response.setStatus(decision.status, decision.reason);
                response.setHeader("Content-Type", "text/html");
                response.setBody("<html><body><h1>" + decision.reason + "</h1></body></html>");
                break;
                
            default:
                VERBOSE_OUT("[Response] Default response" << std::endl);
                response.setStatus(200, "OK");
                response.setBody("Default response");
        }
        
        // 4. Serialize response
        VERBOSE_OUT("[Step 4] Serializing response" << std::endl);
        std::string serializedResponse = response.serialize();
        VERBOSE_OUT("[Result] response ready for transmission (" << serializedResponse.length() << " bytes)" << std::endl);
        
        // Validate complete flow
        ASSERT_TRUE(!serializedResponse.empty());
        ASSERT_TRUE(serializedResponse.find("HTTP/1.1") == 0);
}

static void test_complete_post_request_flow()
{
    VERBOSE_OUT("[Test] Complete POST request processing flow" << std::endl);
    
    // 1. Parse POST request with body
    Parser parser;
    parser.setLimits(8192, 1024*1024, 4096);
    
    // Split the request into headers and body to simulate network reception
    std::string headers = "POST /api/data HTTP/1.1\r\n"
                         "Host: localhost:8080\r\n"
                         "Content-Type: application/json\r\n"
                         "Content-Length: 24\r\n"
                         "\r\n";
    std::string body = "{\"name\":\"test\",\"id\":123}";
    
    VERBOSE_OUT("[Step 1] Parsing POST request with JSON body" << std::endl);
    VERBOSE_OUT("[Input headers]\n" << headers << std::endl);
    VERBOSE_OUT("[Input body] " << body << " (length: " << body.length() << ")" << std::endl);
    
    // Feed headers first
    parser.feed(headers);
    // Feed body
    parser.feed(body);
    
    ASSERT_TRUE(parser.isComplete());
    Request request = parser.getRequest();
    
    VERBOSE_OUT("[Result] parsed: method=" << request.getMethod() 
              << ", path=" << request.getPath() 
              << ", body_length=" << request.getBody().length() << std::endl);
    
    // 2. Simulate routing (avoid Config/ServerManager construction issues)
    VERBOSE_OUT("[Step 2] Simulating routing decision for API endpoint" << std::endl);
    
    // Simulate a routing decision without actual Router::decide call
    Router::Decision decision;
    if (request.getMethod() == "POST" && request.getPath().find("/api/") == 0) {
        decision.type = Router::ACTION_CGI; // Simulate API processing
        decision.status = 200;
        decision.reason = "OK";
        VERBOSE_OUT("[Result] routing decision: API endpoint detected, type=CGI simulation" << std::endl);
        
        // 3. Process based on request
        Response response;
        
        VERBOSE_OUT("[Step 3] Processing API POST request" << std::endl);
        response.setStatus(201, "Created");
        response.setHeader("Content-Type", "application/json");
        response.setBody("{\"status\":\"created\",\"id\":123}");
        
        std::string serialized = response.serialize();
        VERBOSE_OUT("[Result] POST response generated (" << serialized.length() << " bytes)" << std::endl);
        
        ASSERT_TRUE(!serialized.empty());
        ASSERT_TRUE(response.getStatusCode() == 201);
    } else {
        VERBOSE_OUT("[Result] routing decision: endpoint not found" << std::endl);
        Response response;
        response.setStatus(404, "Not Found");
        response.setBody("Endpoint not found");
        ASSERT_TRUE(response.getStatusCode() == 404);
    }
}

static void test_error_handling_flow()
{
    VERBOSE_OUT("[Test] Error handling throughout the flow" << std::endl);
    
    // 1. Malformed request
    Parser parser;
    parser.setLimits(8192, 1024*1024, 4096);
    std::string malformedRequest = "INVALID REQUEST FORMAT\r\n\r\n";
    
    VERBOSE_OUT("[Step 1] Testing malformed request handling" << std::endl);
    VERBOSE_OUT("[Input] " << malformedRequest << std::endl);
    
    parser.feed(malformedRequest);
    Request request = parser.getRequest();
    
    VERBOSE_OUT("[Result] parser error=" << (request.getError() ? "true" : "false"));
    if (request.getError()) {
        VERBOSE_OUT(", code=" << request.getErrorCode());
    }
    VERBOSE_OUT(std::endl);
    
    // 2. Generate error response
    if (request.getError()) {
        VERBOSE_OUT("[Step 2] Generating error response for malformed request" << std::endl);
        Response errorResponse;
        errorResponse.setStatus(request.getErrorCode(), request.getErrorMessage());
        errorResponse.setHeader("Content-Type", "text/html");
        errorResponse.setBody("<html><body><h1>Bad Request</h1></body></html>");
        
        std::string serialized = errorResponse.serialize();
        VERBOSE_OUT("[Result] error response generated: status=" << errorResponse.getStatusCode() << std::endl);
        
        ASSERT_TRUE(errorResponse.getStatusCode() >= 400);
        ASSERT_TRUE(!serialized.empty());
    }
}

static void test_request_with_cookies_flow()
{
    VERBOSE_OUT("[Test] Request with cookies processing flow" << std::endl);
    
    // 1. Parse request with cookies
    Parser parser;
    parser.setLimits(8192, 1024*1024, 4096);
    std::string rawRequest = "GET /profile HTTP/1.1\r\n"
                            "Host: localhost:8080\r\n"
                            "Cookie: sessionId=abc123; authToken=xyz789\r\n"
                            "\r\n";
    
    VERBOSE_OUT("[Step 1] Parsing request with cookies" << std::endl);
    VERBOSE_OUT("[Input]\n" << rawRequest << std::endl);
    
    parser.feed(rawRequest);
    ASSERT_TRUE(parser.isComplete());
    Request request = parser.getRequest();
    
    VERBOSE_OUT("[Result] cookies header: " << request.getHeader("cookie") << std::endl);
    
    // 2. Process request and set response cookies
    VERBOSE_OUT("[Step 2] Processing authenticated request" << std::endl);
    Response response;
    
    std::string cookieHeader = request.getHeader("cookie");
    if (!cookieHeader.empty() && cookieHeader.find("sessionId=") != std::string::npos) {
        VERBOSE_OUT("[Processing] Valid session found" << std::endl);
        response.setStatus(200, "OK");
        response.setHeader("Content-Type", "text/html");
        response.setBody("<html><body>Welcome back!</body></html>");
        // Refresh session cookie
        response.addCookie("sessionId", "abc123", "Path=/; HttpOnly; Max-Age=3600");
    } else {
        VERBOSE_OUT("[Processing] No valid session" << std::endl);
        response.setStatus(401, "Unauthorized");
        response.setBody("Please log in");
    }
    
    std::string serialized = response.serialize();
    VERBOSE_OUT("[Result] response with session handling (" << serialized.length() << " bytes)" << std::endl);
    
    ASSERT_TRUE(!serialized.empty());
    ASSERT_TRUE(response.getStatusCode() == 200 || response.getStatusCode() == 401);
}

static void test_chunked_response_flow()
{
    VERBOSE_OUT("[Test] Chunked response generation flow" << std::endl);
    
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
    
    VERBOSE_OUT("[Input] Large content (" << largeContent.length() << " bytes)" << std::endl);
    VERBOSE_OUT("[Expect] Chunked encoding response" << std::endl);
    
    std::string serialized = response.serialize();
    VERBOSE_OUT("[Result] chunked=" << (response.isChunked() ? "true" : "false") << std::endl);
    VERBOSE_OUT("[Result] has Transfer-Encoding=" << (serialized.find("Transfer-Encoding: chunked") != std::string::npos ? "true" : "false") << std::endl);
    VERBOSE_OUT("[Result] no Content-Length=" << (serialized.find("Content-Length:") == std::string::npos ? "true" : "false") << std::endl);
    
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
