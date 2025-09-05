#include "test_harness.hpp"
#include "../include/Response.hpp"
#include "../include/Router.hpp"
#include "RequestExtended.hpp"
// Note: Don't include CGI.hpp directly to avoid Request class redefinition
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>

// Forward declaration of CGI class to avoid header conflicts
class CGI {
public:
    static Response run(const Router::Decision &decision, const Request &req);
};

// Helper function to create a simple test script
static void createTestScript(const std::string& path, const std::string& content) {
    std::ofstream file(path.c_str());
    file << content;
    file.close();
    chmod(path.c_str(), 0755); // Make executable
}

// Helper function to clean up test files
static void cleanupTestFile(const std::string& path) {
    unlink(path.c_str());
}

// Test 1: Invalid decision type should return 500 error
static void test_cgi_invalid_decision_type() {
    std::cout << "[Test] CGI with invalid decision type" << std::endl;
    
    Request req;
    req.setMethod("GET");
    req.setUri("/test.py");
    
    Router::Decision decision;
    decision.type = Router::ACTION_STATIC; // Wrong type for CGI
    
    Response response = CGI::run(decision, req);
    
    std::cout << "[Expect] Status 500, error message about invalid action type" << std::endl;
    ASSERT_EQ(response.getStatusCode(), 500);
    ASSERT_TRUE(response.getBody().find("Invalid action type") != std::string::npos);
}

// Test 2: Basic CGI script execution with GET request
static void test_cgi_basic_get_request() {
    std::cout << "[Test] Basic CGI GET request" << std::endl;
    
    // Create a simple test script
    std::string scriptPath = "/tmp/test_basic.py";
    createTestScript(scriptPath, 
        "#!/usr/bin/env python3\n"
        "print('Content-Type: text/plain')\n"
        "print()\n"
        "print('Hello from CGI!')\n");
    
    Request req;
    req.setMethod("GET");
    req.setUri("/cgi-bin/test_basic.py");
    req.setPath("/cgi-bin/test_basic.py");
    req.setHeader("Host", "localhost");
    req.setHeader("User-Agent", "TestClient/1.0");
    
    Router::Decision decision(
        Router::ACTION_CGI,
        scriptPath,                    // fsPath
        "test_basic.py",              // relPath
        "/cgi-bin/",                  // mountUri
        "/var/www",                   // root
        ".py",                        // cgiExt
        "/usr/bin/python3"            // cgiInterpreter
    );
    
    Response response = CGI::run(decision, req);
    
    std::cout << "[Expect] Status 200, Content-Type text/plain, body 'Hello from CGI!'" << std::endl;
    ASSERT_EQ(response.getStatusCode(), 200);
    ASSERT_EQ(response.getHeaders().find("Content-Type")->second, std::string("text/plain"));
    ASSERT_TRUE(response.getBody().find("Hello from CGI!") != std::string::npos);
    
    cleanupTestFile(scriptPath);
}

// Test 3: CGI script with query string
static void test_cgi_with_query_string() {
    std::cout << "[Test] CGI with query string" << std::endl;
    
    std::string scriptPath = "/tmp/test_query.py";
    createTestScript(scriptPath, 
        "#!/usr/bin/env python3\n"
        "import os\n"
        "print('Content-Type: text/plain')\n"
        "print()\n"
        "print('QUERY_STRING=' + os.environ.get('QUERY_STRING', ''))\n");
    
    Request req;
    req.setMethod("GET");
    req.setUri("/cgi-bin/test_query.py?name=test&value=123");
    req.setPath("/cgi-bin/test_query.py");
    req.setQuery("name=test&value=123");
    
    Router::Decision decision(
        Router::ACTION_CGI,
        scriptPath + "?name=test&value=123",
        "test_query.py",
        "/cgi-bin/",
        "/var/www",
        ".py",
        "/usr/bin/python3"
    );
    
    Response response = CGI::run(decision, req);
    
    std::cout << "[Expect] Status 200, body contains 'QUERY_STRING=name=test&value=123'" << std::endl;
    ASSERT_EQ(response.getStatusCode(), 200);
    ASSERT_TRUE(response.getBody().find("QUERY_STRING=name=test&value=123") != std::string::npos);
    
    cleanupTestFile(scriptPath);
}

// Test 4: CGI script with POST data
static void test_cgi_post_request() {
    std::cout << "[Test] CGI POST request with body" << std::endl;
    
    std::string scriptPath = "/tmp/test_post.py";
    createTestScript(scriptPath, 
        "#!/usr/bin/env python3\n"
        "import sys\n"
        "import os\n"
        "content_length = int(os.environ.get('CONTENT_LENGTH', '0'))\n"
        "data = sys.stdin.read(content_length) if content_length > 0 else ''\n"
        "print('Content-Type: text/plain')\n"
        "print()\n"
        "print('METHOD=' + os.environ.get('REQUEST_METHOD', ''))\n"
        "print('DATA=' + data)\n");
    
    Request req;
    req.setMethod("POST");
    req.setUri("/cgi-bin/test_post.py");
    req.setPath("/cgi-bin/test_post.py");
    req.setHeader("Content-Type", "application/x-www-form-urlencoded");
    req.setHeader("Content-Length", "13");
    req.appendToBody("hello=world");
    
    Router::Decision decision(
        Router::ACTION_CGI,
        scriptPath,
        "test_post.py",
        "/cgi-bin/",
        "/var/www",
        ".py",
        "/usr/bin/python3"
    );
    
    Response response = CGI::run(decision, req);
    
    std::cout << "[Expect] Status 200, body contains 'METHOD=POST' and 'DATA=hello=world'" << std::endl;
    ASSERT_EQ(response.getStatusCode(), 200);
    ASSERT_TRUE(response.getBody().find("METHOD=POST") != std::string::npos);
    ASSERT_TRUE(response.getBody().find("DATA=hello=world") != std::string::npos);
    
    cleanupTestFile(scriptPath);
}

// Test 5: CGI script that returns custom status
static void test_cgi_custom_status() {
    std::cout << "[Test] CGI script with custom status" << std::endl;
    
    std::string scriptPath = "/tmp/test_status.py";
    createTestScript(scriptPath, 
        "#!/usr/bin/env python3\n"
        "print('Status: 201 Created')\n"
        "print('Content-Type: text/plain')\n"
        "print()\n"
        "print('Resource created successfully')\n");
    
    Request req;
    req.setMethod("POST");
    req.setUri("/cgi-bin/test_status.py");
    
    Router::Decision decision(
        Router::ACTION_CGI,
        scriptPath,
        "test_status.py",
        "/cgi-bin/",
        "/var/www",
        ".py",
        "/usr/bin/python3"
    );
    
    Response response = CGI::run(decision, req);
    
    std::cout << "[Expect] Status 201, body 'Resource created successfully'" << std::endl;
    ASSERT_EQ(response.getStatusCode(), 201);
    ASSERT_TRUE(response.getBody().find("Resource created successfully") != std::string::npos);
    
    cleanupTestFile(scriptPath);
}

// Test 6: CGI script with redirect (Location header)
static void test_cgi_redirect() {
    std::cout << "[Test] CGI script with redirect" << std::endl;
    
    std::string scriptPath = "/tmp/test_redirect.py";
    createTestScript(scriptPath, 
        "#!/usr/bin/env python3\n"
        "print('Location: https://example.com/redirected')\n"
        "print('Content-Type: text/plain')\n"
        "print()\n"
        "print('Redirecting...')\n");
    
    Request req;
    req.setMethod("GET");
    req.setUri("/cgi-bin/test_redirect.py");
    
    Router::Decision decision(
        Router::ACTION_CGI,
        scriptPath,
        "test_redirect.py",
        "/cgi-bin/",
        "/var/www",
        ".py",
        "/usr/bin/python3"
    );
    
    Response response = CGI::run(decision, req);
    
    std::cout << "[Expect] Status 302, Location header set" << std::endl;
    ASSERT_EQ(response.getStatusCode(), 302);
    ASSERT_EQ(response.getHeaders().find("Location")->second, std::string("https://example.com/redirected"));
    
    cleanupTestFile(scriptPath);
}

// Test 7: CGI script with environment variables
static void test_cgi_environment_variables() {
    std::cout << "[Test] CGI environment variables" << std::endl;
    
    std::string scriptPath = "/tmp/test_env.py";
    createTestScript(scriptPath, 
        "#!/usr/bin/env python3\n"
        "import os\n"
        "print('Content-Type: text/plain')\n"
        "print()\n"
        "print('GATEWAY_INTERFACE=' + os.environ.get('GATEWAY_INTERFACE', ''))\n"
        "print('SERVER_PROTOCOL=' + os.environ.get('SERVER_PROTOCOL', ''))\n"
        "print('REQUEST_METHOD=' + os.environ.get('REQUEST_METHOD', ''))\n"
        "print('SCRIPT_NAME=' + os.environ.get('SCRIPT_NAME', ''))\n"
        "print('HTTP_HOST=' + os.environ.get('HTTP_HOST', ''))\n");
    
    Request req;
    req.setMethod("GET");
    req.setUri("/cgi-bin/test_env.py");
    req.setPath("/cgi-bin/test_env.py");
    req.setHeader("Host", "testserver.com");
    req.setProtocol("HTTP/1.1");
    req.setServerName("testserver.com");
    req.setServerPort("8080");
    
    Router::Decision decision(
        Router::ACTION_CGI,
        scriptPath,
        "test_env.py",
        "/cgi-bin/",
        "/var/www",
        ".py",
        "/usr/bin/python3"
    );
    
    Response response = CGI::run(decision, req);
    
    std::cout << "[Expect] Status 200, proper CGI environment variables" << std::endl;
    ASSERT_EQ(response.getStatusCode(), 200);
    ASSERT_TRUE(response.getBody().find("GATEWAY_INTERFACE=CGI/1.1") != std::string::npos);
    ASSERT_TRUE(response.getBody().find("SERVER_PROTOCOL=HTTP/1.1") != std::string::npos);
    ASSERT_TRUE(response.getBody().find("REQUEST_METHOD=GET") != std::string::npos);
    ASSERT_TRUE(response.getBody().find("HTTP_HOST=testserver.com") != std::string::npos);
    
    cleanupTestFile(scriptPath);
}

// Test 8: CGI script with PATH_INFO
static void test_cgi_path_info() {
    std::cout << "[Test] CGI with PATH_INFO" << std::endl;
    
    std::string scriptPath = "/tmp/test_pathinfo.py";
    createTestScript(scriptPath, 
        "#!/usr/bin/env python3\n"
        "import os\n"
        "print('Content-Type: text/plain')\n"
        "print()\n"
        "print('PATH_INFO=' + os.environ.get('PATH_INFO', ''))\n");
    
    Request req;
    req.setMethod("GET");
    req.setUri("/cgi-bin/test_pathinfo.py/extra/path/info");
    req.setPath("/cgi-bin/test_pathinfo.py/extra/path/info");
    
    Router::Decision decision(
        Router::ACTION_CGI,
        scriptPath,
        "test_pathinfo.py/extra/path/info",
        "/cgi-bin/",
        "/var/www",
        ".py",
        "/usr/bin/python3"
    );
    
    Response response = CGI::run(decision, req);
    
    std::cout << "[Expect] Status 200, PATH_INFO='/extra/path/info'" << std::endl;
    ASSERT_EQ(response.getStatusCode(), 200);
    ASSERT_TRUE(response.getBody().find("PATH_INFO=/extra/path/info") != std::string::npos);
    
    cleanupTestFile(scriptPath);
}

// Test 9: CGI script execution failure (non-existent script)
static void test_cgi_script_not_found() {
    std::cout << "[Test] CGI script not found" << std::endl;
    
    Request req;
    req.setMethod("GET");
    req.setUri("/cgi-bin/nonexistent.py");
    
    Router::Decision decision(
        Router::ACTION_CGI,
        "/tmp/nonexistent_script.py",  // This file doesn't exist
        "nonexistent.py",
        "/cgi-bin/",
        "/var/www",
        ".py",
        "/usr/bin/python3"
    );
    
    Response response = CGI::run(decision, req);
    
    std::cout << "[Expect] Status 500, error about script execution" << std::endl;
    ASSERT_EQ(response.getStatusCode(), 500);
    ASSERT_TRUE(response.getBody().find("Internal Server Error") != std::string::npos);
}

// Test 10: CGI script with no interpreter (direct execution)
static void test_cgi_direct_execution() {
    std::cout << "[Test] CGI direct execution (no interpreter)" << std::endl;
    
    std::string scriptPath = "/tmp/test_direct.sh";
    createTestScript(scriptPath, 
        "#!/bin/bash\n"
        "echo 'Content-Type: text/plain'\n"
        "echo\n"
        "echo 'Hello from shell script!'\n");
    
    Request req;
    req.setMethod("GET");
    req.setUri("/cgi-bin/test_direct.sh");
    
    Router::Decision decision(
        Router::ACTION_CGI,
        scriptPath,
        "test_direct.sh",
        "/cgi-bin/",
        "/var/www",
        ".sh",
        ""  // No interpreter, direct execution
    );
    
    Response response = CGI::run(decision, req);
    
    std::cout << "[Expect] Status 200, body 'Hello from shell script!'" << std::endl;
    ASSERT_EQ(response.getStatusCode(), 200);
    ASSERT_TRUE(response.getBody().find("Hello from shell script!") != std::string::npos);
    
    cleanupTestFile(scriptPath);
}

int main() {
    std::cout << COLOR_BLUE << "Running CGI Test Suite" << COLOR_RESET << std::endl;
    std::cout << "======================================" << std::endl;

    RUN_TEST(test_cgi_invalid_decision_type);
    RUN_TEST(test_cgi_basic_get_request);
    RUN_TEST(test_cgi_with_query_string);
    RUN_TEST(test_cgi_post_request);
    RUN_TEST(test_cgi_custom_status);
    RUN_TEST(test_cgi_redirect);
    RUN_TEST(test_cgi_environment_variables);
    RUN_TEST(test_cgi_path_info);
    RUN_TEST(test_cgi_script_not_found);
    RUN_TEST(test_cgi_direct_execution);

    return finish_tests();
}
