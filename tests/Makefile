# Makefile for CGI Test Suite

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I../include -I.
SRCDIR = ../src
TESTDIR = .

# Source files
RESPONSE_SRC = $(SRCDIR)/Response.cpp
CGI_SRC = $(SRCDIR)/CGI.cpp

# Test files
REQUEST_EXT_SRC = $(TESTDIR)/RequestExtended.cpp
TEST_CGI_SRC = $(TESTDIR)/TestCGI.cpp
TEST_CGI_UNIT_SRC = $(TESTDIR)/TestCGIUnit.cpp

# Executables
TEST_CGI_EXE = test_cgi
TEST_CGI_UNIT_EXE = test_cgi_unit

.PHONY: all clean test_unit test_integration test

all: $(TEST_CGI_UNIT_EXE) $(TEST_CGI_EXE)

# Unit tests (no actual CGI execution)
$(TEST_CGI_UNIT_EXE): $(TEST_CGI_UNIT_SRC) $(REQUEST_EXT_SRC) $(RESPONSE_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Integration tests (requires actual CGI execution)
$(TEST_CGI_EXE): $(TEST_CGI_SRC) $(REQUEST_EXT_SRC) $(RESPONSE_SRC) $(CGI_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Run unit tests only
test_unit: $(TEST_CGI_UNIT_EXE)
	@echo "Running CGI Unit Tests..."
	./$(TEST_CGI_UNIT_EXE)

# Run integration tests (may fail if CGI environment not set up)
test_integration: $(TEST_CGI_EXE)
	@echo "Running CGI Integration Tests..."
	@echo "NOTE: These tests require Python3 to be installed and executable scripts to work"
	./$(TEST_CGI_EXE)

# Run all tests
test: test_unit test_integration

# Clean up
clean:
	rm -f $(TEST_CGI_EXE) $(TEST_CGI_UNIT_EXE)
	rm -f /tmp/test_*.py /tmp/test_*.sh

help:
	@echo "Available targets:"
	@echo "  all              - Build all test executables"
	@echo "  test_unit        - Run unit tests (safe, no script execution)"
	@echo "  test_integration - Run integration tests (requires CGI environment)"
	@echo "  test             - Run all tests"
	@echo "  clean            - Remove executables and test files"
	@echo "  help             - Show this help message"
