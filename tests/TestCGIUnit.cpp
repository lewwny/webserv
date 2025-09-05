#include "test_harness.hpp"
#include "RequestExtended.hpp"
#include "../include/Response.hpp"
#include "../include/Router.hpp"
#include <iostream>

// Mock CGI class for unit testing without actual script execution
class MockCGI {
public:
    // Test the static helper functions that would be in CGI.cpp
    static void splitFsPathQuery(const std::string &fsPath, std::string &scriptPath, std::string &query) {
        size_t pos = fsPath.find("?");
        if (pos == std::string::npos) {
            scriptPath = fsPath;
            query = "";
        } else {
            scriptPath = fsPath.substr(0, pos);
            query = fsPath.substr(pos + 1);
        }
    }
    
    static std::string computePathInfo(const std::string &relPath, const std::string &cgiExt) {
        if (cgiExt.empty()) {
            return std::string();
        }
        size_t pos = relPath.rfind(cgiExt);
        if (pos == std::string::npos) {
            return std::string();
        }
        size_t pathInfoStart = pos + cgiExt.size();
        if (pathInfoStart >= relPath.size()) {
            return std::string();
        }
        std::string pathInfo = relPath.substr(pathInfoStart);
        if (!pathInfo.empty() && pathInfo[0] != '/') {
            pathInfo.insert(pathInfo.begin(), '/');
        }
        return pathInfo;
    }
    
    static bool parseCgiOutput(Response &res, const std::string &raw) {
        size_t header_end = raw.find("\r\n\r\n");
        size_t sep_len = 4;
        if (header_end == std::string::npos) {
            header_end = raw.find("\n\n");
            sep_len = 2;
        }
        if (header_end == std::string::npos)
            return false;

        std::string header = raw.substr(0, header_end);
        std::string body   = raw.substr(header_end + sep_len);

        bool hasStatus = false;
        res.setStatus(200, "OK");

        size_t pos = 0;
        while (pos < header.size()) {
            size_t line_end = header.find('\n', pos);
            if (line_end == std::string::npos)
                line_end = header.size();
            std::string line = header.substr(pos, line_end - pos);
            if (!line.empty() && line[line.size() - 1] == '\r')
                line.erase(line.size() - 1);

            if (!line.empty()) {
                if (line.rfind("Status:", 0) == 0) {
                    size_t sp = line.find(' ');
                    if (sp != std::string::npos) {
                        int status = atoi(line.substr(sp + 1).c_str());
                        res.setStatus(status, "");
                        hasStatus = true;
                    }
                } else {
                    size_t cp = line.find(':');
                    if (cp != std::string::npos) {
                        std::string key = line.substr(0, cp);
                        std::string value = line.substr(cp + 1);
                        size_t i = value.find_first_not_of(" \t");
                        if (i != std::string::npos)
                            value.erase(0, i);
                        res.setHeader(key, value);
                    }
                }
            }
            pos = (line_end == header.size()) ? line_end : line_end + 1;
        }

        const std::map<std::string, std::string>& H = res.getHeaders();
        if (!hasStatus && (H.find("Location") != H.end())) {
            res.setStatus(302, "Found");
        }

        res.setBody(body);
        return true;
    }
};

// Test 1: Split filesystem path and query string
static void test_split_fs_path_query() {
    std::cout << "[Test] Split filesystem path and query string" << std::endl;
    
    std::string scriptPath, query;
    
    // Test with query string
    MockCGI::splitFsPathQuery("/tmp/script.py?name=test&value=123", scriptPath, query);
    std::cout << "[Expect] scriptPath='/tmp/script.py', query='name=test&value=123'" << std::endl;
    ASSERT_EQ(scriptPath, std::string("/tmp/script.py"));
    ASSERT_EQ(query, std::string("name=test&value=123"));
    
    // Test without query string
    MockCGI::splitFsPathQuery("/tmp/script.py", scriptPath, query);
    std::cout << "[Expect] scriptPath='/tmp/script.py', query=''" << std::endl;
    ASSERT_EQ(scriptPath, std::string("/tmp/script.py"));
    ASSERT_EQ(query, std::string(""));
    
    // Test empty path
    MockCGI::splitFsPathQuery("", scriptPath, query);
    std::cout << "[Expect] scriptPath='', query=''" << std::endl;
    ASSERT_EQ(scriptPath, std::string(""));
    ASSERT_EQ(query, std::string(""));
}

// Test 2: Compute PATH_INFO
static void test_compute_path_info() {
    std::cout << "[Test] Compute PATH_INFO from relative path and CGI extension" << std::endl;
    
    // Test with PATH_INFO
    std::string pathInfo = MockCGI::computePathInfo("script.py/extra/path", ".py");
    std::cout << "[Expect] pathInfo='/extra/path'" << std::endl;
    ASSERT_EQ(pathInfo, std::string("/extra/path"));
    
    // Test without PATH_INFO
    pathInfo = MockCGI::computePathInfo("script.py", ".py");
    std::cout << "[Expect] pathInfo=''" << std::endl;
    ASSERT_EQ(pathInfo, std::string(""));
    
    // Test with empty extension
    pathInfo = MockCGI::computePathInfo("script.py/extra/path", "");
    std::cout << "[Expect] pathInfo=''" << std::endl;
    ASSERT_EQ(pathInfo, std::string(""));
    
    // Test with no extension match
    pathInfo = MockCGI::computePathInfo("script.sh/extra/path", ".py");
    std::cout << "[Expect] pathInfo=''" << std::endl;
    ASSERT_EQ(pathInfo, std::string(""));
    
    // Test PATH_INFO without leading slash
    pathInfo = MockCGI::computePathInfo("script.pyextra/path", ".py");
    std::cout << "[Expect] pathInfo='/extra/path'" << std::endl;
    ASSERT_EQ(pathInfo, std::string("/extra/path"));
}

// Test 3: Parse CGI output with headers and body
static void test_parse_cgi_output_basic() {
    std::cout << "[Test] Parse basic CGI output" << std::endl;
    
    std::string cgiOutput = 
        "Content-Type: text/html\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Hello, World!";
    
    Response res;
    bool success = MockCGI::parseCgiOutput(res, cgiOutput);
    
    std::cout << "[Expect] Success, status 200, Content-Type text/html, body 'Hello, World!'" << std::endl;
    ASSERT_TRUE(success);
    ASSERT_EQ(res.getStatusCode(), 200);
    ASSERT_EQ(res.getHeaders().find("Content-Type")->second, std::string("text/html"));
    ASSERT_EQ(res.getBody(), std::string("Hello, World!"));
}

// Test 4: Parse CGI output with custom status
static void test_parse_cgi_output_status() {
    std::cout << "[Test] Parse CGI output with custom status" << std::endl;
    
    std::string cgiOutput = 
        "Status: 201 Created\n"
        "Content-Type: application/json\n"
        "\n"
        "{\"message\": \"Resource created\"}";
    
    Response res;
    bool success = MockCGI::parseCgiOutput(res, cgiOutput);
    
    std::cout << "[Expect] Success, status 201, JSON body" << std::endl;
    ASSERT_TRUE(success);
    ASSERT_EQ(res.getStatusCode(), 201);
    ASSERT_EQ(res.getHeaders().find("Content-Type")->second, std::string("application/json"));
    ASSERT_TRUE(res.getBody().find("Resource created") != std::string::npos);
}

// Test 5: Parse CGI output with Location header (redirect)
static void test_parse_cgi_output_redirect() {
    std::cout << "[Test] Parse CGI output with redirect" << std::endl;
    
    std::string cgiOutput = 
        "Location: https://example.com/redirected\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "Redirecting...";
    
    Response res;
    bool success = MockCGI::parseCgiOutput(res, cgiOutput);
    
    std::cout << "[Expect] Success, status 302 (auto-detected), Location header set" << std::endl;
    ASSERT_TRUE(success);
    ASSERT_EQ(res.getStatusCode(), 302);
    ASSERT_EQ(res.getHeaders().find("Location")->second, std::string("https://example.com/redirected"));
    ASSERT_EQ(res.getBody(), std::string("Redirecting..."));
}

// Test 6: Parse malformed CGI output (no headers)
static void test_parse_cgi_output_malformed() {
    std::cout << "[Test] Parse malformed CGI output" << std::endl;
    
    std::string cgiOutput = "This is just text without headers";
    
    Response res;
    bool success = MockCGI::parseCgiOutput(res, cgiOutput);
    
    std::cout << "[Expect] Failure (no header/body separator found)" << std::endl;
    ASSERT_TRUE(!success);
}

// Test 7: Router Decision structure validation
static void test_router_decision_structure() {
    std::cout << "[Test] Router Decision structure" << std::endl;
    
    Router::Decision decision;
    
    // Test default constructor
    std::cout << "[Expect] Default type is ACTION_ERROR" << std::endl;
    ASSERT_EQ(decision.type, Router::ACTION_ERROR);
    
    // Test parameterized constructor
    Router::Decision cgiDecision;
    cgiDecision.type = Router::ACTION_CGI;
    cgiDecision.fsPath = "/var/www/cgi-bin/script.py";
    cgiDecision.relPath = "script.py";
    cgiDecision.mountUri = "/cgi-bin/";
    cgiDecision.root = "/var/www";
    cgiDecision.cgiExt = ".py";
    cgiDecision.cgiInterpreter = "/usr/bin/python3";
    
    std::cout << "[Expect] All fields properly initialized" << std::endl;
    ASSERT_EQ(cgiDecision.type, Router::ACTION_CGI);
    ASSERT_EQ(cgiDecision.fsPath, std::string("/var/www/cgi-bin/script.py"));
    ASSERT_EQ(cgiDecision.relPath, std::string("script.py"));
    ASSERT_EQ(cgiDecision.mountUri, std::string("/cgi-bin/"));
    ASSERT_EQ(cgiDecision.root, std::string("/var/www"));
    ASSERT_EQ(cgiDecision.cgiExt, std::string(".py"));
    ASSERT_EQ(cgiDecision.cgiInterpreter, std::string("/usr/bin/python3"));
}

// Test 8: Request extended functionality
static void test_request_extended_functionality() {
    std::cout << "[Test] Request extended functionality for CGI" << std::endl;
    
    Request req;
    
    // Test protocol
    ASSERT_TRUE(!req.hasProtocol());
    req.setProtocol("HTTP/1.1");
    ASSERT_TRUE(req.hasProtocol());
    ASSERT_EQ(req.getProtocol(), std::string("HTTP/1.1"));
    
    // Test server name
    ASSERT_TRUE(!req.hasServerName());
    req.setServerName("example.com");
    ASSERT_TRUE(req.hasServerName());
    ASSERT_EQ(req.getServerName(), std::string("example.com"));
    
    // Test server port
    ASSERT_TRUE(!req.hasServerPort());
    req.setServerPort("8080");
    ASSERT_TRUE(req.hasServerPort());
    ASSERT_EQ(req.getServerPort(), std::string("8080"));
    
    // Test remote address
    ASSERT_TRUE(!req.hasRemoteAddr());
    req.setRemoteAddr("192.168.1.100");
    ASSERT_TRUE(req.hasRemoteAddr());
    ASSERT_EQ(req.getRemoteAddr(), std::string("192.168.1.100"));
    
    std::cout << "[Expect] All extended Request methods work correctly" << std::endl;
}

// Test 9: Response serialization for CGI
static void test_response_serialization() {
    std::cout << "[Test] Response serialization" << std::endl;
    
    Response res;
    res.setStatus(200, "OK");
    res.setHeader("Content-Type", "text/plain");
    res.setHeader("X-Custom-Header", "test-value");
    res.setBody("Hello, World!");
    
    std::string serialized = res.serialize();
    
    std::cout << "[Expect] Proper HTTP response format" << std::endl;
    ASSERT_TRUE(serialized.find("HTTP/1.1 200 OK") != std::string::npos);
    ASSERT_TRUE(serialized.find("Content-Type: text/plain") != std::string::npos);
    ASSERT_TRUE(serialized.find("X-Custom-Header: test-value") != std::string::npos);
    ASSERT_TRUE(serialized.find("Hello, World!") != std::string::npos);
}

int main() {
    std::cout << COLOR_BLUE << "Running CGI Unit Test Suite" << COLOR_RESET << std::endl;
    std::cout << "===========================================" << std::endl;

    RUN_TEST(test_split_fs_path_query);
    RUN_TEST(test_compute_path_info);
    RUN_TEST(test_parse_cgi_output_basic);
    RUN_TEST(test_parse_cgi_output_status);
    RUN_TEST(test_parse_cgi_output_redirect);
    RUN_TEST(test_parse_cgi_output_malformed);
    RUN_TEST(test_router_decision_structure);
    RUN_TEST(test_request_extended_functionality);
    RUN_TEST(test_response_serialization);

    return finish_tests();
}
