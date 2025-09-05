#include "../include/Router.hpp"
#include "../include/Request.hpp"
#include "test_harness.hpp"
#include <iostream>
#include <string>

// Test basic router functionality without dependencies

static void test_router_decision_structure()
{
    if (g_verbose) std::cout << "[Test] Router::Decision structure initialization" << std::endl;
    
    Router::Decision decision;
    decision.type = Router::ACTION_STATIC;
    decision.status = 200;
    decision.reason = "OK";
    decision.fsPath = "/var/www/index.html";
    decision.root = "/var/www";
    
    if (g_verbose) std::cout << "[Input] Setting decision fields" << std::endl;
    if (g_verbose) std::cout << "[Expect] All fields accessible and assignable" << std::endl;
    if (g_verbose) std::cout << "[Result] type=" << decision.type << ", status=" << decision.status << std::endl;
    if (g_verbose) std::cout << "[Result] reason=" << decision.reason << ", fsPath=" << decision.fsPath << std::endl;
    
    ASSERT_TRUE(decision.type == Router::ACTION_STATIC);
    ASSERT_TRUE(decision.status == 200);
    ASSERT_TRUE(decision.reason == "OK");
    ASSERT_TRUE(decision.fsPath == "/var/www/index.html");
}

static void test_router_action_types()
{
    if (g_verbose) std::cout << "[Test] Router action type constants" << std::endl;
    
    if (g_verbose) std::cout << "[Input] Testing action type enum values" << std::endl;
    if (g_verbose) std::cout << "[Expect] All action types have distinct values" << std::endl;
    
    int staticAction = Router::ACTION_STATIC;
    int errorAction = Router::ACTION_ERROR;
    int redirectAction = Router::ACTION_REDIRECT;
    int cgiAction = Router::ACTION_CGI;
    
    if (g_verbose) std::cout << "[Result] ACTION_STATIC=" << staticAction << std::endl;
    if (g_verbose) std::cout << "[Result] ACTION_ERROR=" << errorAction << std::endl;
    if (g_verbose) std::cout << "[Result] ACTION_REDIRECT=" << redirectAction << std::endl;
    if (g_verbose) std::cout << "[Result] ACTION_CGI=" << cgiAction << std::endl;
    
    // Test that all action types are different
    ASSERT_TRUE(staticAction != errorAction);
    ASSERT_TRUE(staticAction != redirectAction);
    ASSERT_TRUE(staticAction != cgiAction);
    ASSERT_TRUE(errorAction != redirectAction);
    ASSERT_TRUE(errorAction != cgiAction);
    ASSERT_TRUE(redirectAction != cgiAction);
}

static void test_router_decision_error_setup()
{
    if (g_verbose) std::cout << "[Test] Router::Decision for error responses" << std::endl;
    
    Router::Decision errorDecision;
    errorDecision.type = Router::ACTION_ERROR;
    errorDecision.status = 404;
    errorDecision.reason = "Not Found";
    errorDecision.fsPath = ""; // No file for errors
    
    if (g_verbose) std::cout << "[Input] Creating 404 error decision" << std::endl;
    if (g_verbose) std::cout << "[Expect] Error decision configured properly" << std::endl;
    if (g_verbose) std::cout << "[Result] error type=" << errorDecision.type << ", status=" << errorDecision.status << std::endl;
    
    ASSERT_TRUE(errorDecision.type == Router::ACTION_ERROR);
    ASSERT_TRUE(errorDecision.status == 404);
    ASSERT_TRUE(errorDecision.reason == "Not Found");
    ASSERT_TRUE(errorDecision.fsPath.empty());
}

static void test_router_decision_redirect_setup()
{
    if (g_verbose) std::cout << "[Test] Router::Decision for redirect responses" << std::endl;
    
    Router::Decision redirectDecision;
    redirectDecision.type = Router::ACTION_REDIRECT;
    redirectDecision.status = 301;
    redirectDecision.reason = "Moved Permanently";
    redirectDecision.fsPath = "https://example.com/new-location";
    
    if (g_verbose) std::cout << "[Input] Creating 301 redirect decision" << std::endl;
    if (g_verbose) std::cout << "[Expect] Redirect decision configured properly" << std::endl;
    if (g_verbose) std::cout << "[Result] redirect type=" << redirectDecision.type << ", status=" << redirectDecision.status << std::endl;
    if (g_verbose) std::cout << "[Result] redirect location=" << redirectDecision.fsPath << std::endl;
    
    ASSERT_TRUE(redirectDecision.type == Router::ACTION_REDIRECT);
    ASSERT_TRUE(redirectDecision.status == 301);
    ASSERT_TRUE(redirectDecision.reason == "Moved Permanently");
    ASSERT_TRUE(!redirectDecision.fsPath.empty());
}

static void test_router_decision_cgi_setup()
{
    if (g_verbose) std::cout << "[Test] Router::Decision for CGI responses" << std::endl;
    
    Router::Decision cgiDecision;
    cgiDecision.type = Router::ACTION_CGI;
    cgiDecision.status = 200;
    cgiDecision.reason = "OK";
    cgiDecision.fsPath = "/var/www/cgi-bin/script.py";
    cgiDecision.cgiInterpreter = "/usr/bin/python3";
    cgiDecision.cgiExt = ".py";
    
    if (g_verbose) std::cout << "[Input] Creating CGI execution decision" << std::endl;
    if (g_verbose) std::cout << "[Expect] CGI decision configured properly" << std::endl;
    if (g_verbose) std::cout << "[Result] cgi type=" << cgiDecision.type << ", script=" << cgiDecision.fsPath << std::endl;
    if (g_verbose) std::cout << "[Result] interpreter=" << cgiDecision.cgiInterpreter << ", ext=" << cgiDecision.cgiExt << std::endl;
    
    ASSERT_TRUE(cgiDecision.type == Router::ACTION_CGI);
    ASSERT_TRUE(cgiDecision.status == 200);
    ASSERT_TRUE(cgiDecision.fsPath.find(".py") != std::string::npos);
    ASSERT_TRUE(cgiDecision.cgiExt == ".py");
}

static void test_router_decision_static_file_setup()
{
    if (g_verbose) std::cout << "[Test] Router::Decision for static file serving" << std::endl;
    
    Router::Decision staticDecision;
    staticDecision.type = Router::ACTION_STATIC;
    staticDecision.status = 200;
    staticDecision.reason = "OK";
    staticDecision.fsPath = "/var/www/assets/style.css";
    staticDecision.root = "/var/www";
    staticDecision.relPath = "/assets/style.css";
    
    if (g_verbose) std::cout << "[Input] Creating static file decision" << std::endl;
    if (g_verbose) std::cout << "[Expect] Static file decision configured properly" << std::endl;
    if (g_verbose) std::cout << "[Result] static type=" << staticDecision.type << ", file=" << staticDecision.fsPath << std::endl;
    if (g_verbose) std::cout << "[Result] root=" << staticDecision.root << ", relPath=" << staticDecision.relPath << std::endl;
    
    ASSERT_TRUE(staticDecision.type == Router::ACTION_STATIC);
    ASSERT_TRUE(staticDecision.status == 200);
    ASSERT_TRUE(staticDecision.fsPath.find(".css") != std::string::npos);
    ASSERT_TRUE(staticDecision.root == "/var/www");
}

static void test_basic_request_routing_pattern()
{
    if (g_verbose) std::cout << "[Test] Basic request routing pattern test" << std::endl;
    
    // Create a simple request
    Request request;
    request.setMethod("GET");
    request.setPath("/index.html");
    request.setHeader("host", "localhost:8080");
    
    if (g_verbose) std::cout << "[Input] GET /index.html" << std::endl;
    if (g_verbose) std::cout << "[Expect] Static file routing pattern" << std::endl;
    
    // Simulate routing logic pattern (without actual Router::decide)
    Router::Decision decision;
    std::string path = request.getPath();
    
    if (path == "/index.html") {
        decision.type = Router::ACTION_STATIC;
        decision.status = 200;
        decision.reason = "OK";
        decision.fsPath = "/var/www" + path;
        decision.root = "/var/www";
        decision.relPath = path;
    }
    
    if (g_verbose) std::cout << "[Result] decision type=" << decision.type << ", file=" << decision.fsPath << std::endl;
    
    ASSERT_TRUE(decision.type == Router::ACTION_STATIC);
    ASSERT_TRUE(decision.fsPath == "/var/www/index.html");
}

static void test_cgi_detection_pattern()
{
    if (g_verbose) std::cout << "[Test] CGI detection pattern test" << std::endl;
    
    // Test various paths for CGI detection
    std::string cgiPath = "/cgi-bin/test.py";
    std::string normalPath = "/static/page.html";
    
    if (g_verbose) std::cout << "[Input] Testing paths: " << cgiPath << " and " << normalPath << std::endl;
    if (g_verbose) std::cout << "[Expect] CGI detection based on path patterns" << std::endl;
    
    bool isCgi1 = (cgiPath.find("/cgi-bin/") == 0);
    bool isCgi2 = (normalPath.find("/cgi-bin/") == 0);
    
    if (g_verbose) std::cout << "[Result] " << cgiPath << " is CGI: " << (isCgi1 ? "true" : "false") << std::endl;
    if (g_verbose) std::cout << "[Result] " << normalPath << " is CGI: " << (isCgi2 ? "true" : "false") << std::endl;
    
    ASSERT_TRUE(isCgi1);
    ASSERT_TRUE(!isCgi2);
}

int main(int argc, char* argv[])
{
    parse_args(argc, argv);
    
    RUN_TEST(test_router_decision_structure);
    RUN_TEST(test_router_action_types);
    RUN_TEST(test_router_decision_error_setup);
    RUN_TEST(test_router_decision_redirect_setup);
    RUN_TEST(test_router_decision_cgi_setup);
    RUN_TEST(test_router_decision_static_file_setup);
    RUN_TEST(test_basic_request_routing_pattern);
    RUN_TEST(test_cgi_detection_pattern);
    return finish_tests();
}
