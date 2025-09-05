# CGI Test Suite

This directory contains a comprehensive test suite for the CGI functionality of the webserver project.

## Files Overview

### Core Test Files
- **`TestCGIUnit.cpp`** - Unit tests that test CGI helper functions without actual script execution
- **`TestCGI.cpp`** - Integration tests that execute real CGI scripts (requires Python3)
- **`test_harness.hpp`** - Test framework with assertion macros and colored output

### Supporting Files
- **`Router.hpp`** - Temporary Router class definition for testing
- **`RequestExtended.hpp/cpp`** - Extended Request class with additional methods needed by CGI
- **`Makefile`** - Build system for compiling and running tests

## Test Categories

### Unit Tests (`TestCGIUnit.cpp`)
These tests are safe to run in any environment and test:

1. **Path/Query Parsing** - `splitFsPathQuery()` function
2. **PATH_INFO Computation** - `computePathInfo()` function  
3. **CGI Output Parsing** - `parseCgiOutput()` function with various scenarios
4. **Router Decision Structure** - Validation of Router::Decision fields
5. **Extended Request Methods** - Additional methods for CGI environment
6. **Response Serialization** - HTTP response formatting

### Integration Tests (`TestCGI.cpp`)
These tests require a working CGI environment and test:

1. **Invalid Decision Type** - Error handling for wrong action types
2. **Basic GET Request** - Simple CGI script execution
3. **Query String Handling** - CGI scripts with URL parameters
4. **POST Request** - CGI scripts receiving POST data
5. **Custom Status Codes** - CGI scripts returning non-200 status
6. **HTTP Redirects** - CGI scripts with Location headers
7. **Environment Variables** - Proper CGI environment setup
8. **PATH_INFO** - Extra path information handling
9. **Script Not Found** - Error handling for missing scripts
10. **Direct Execution** - Scripts without interpreters (shell scripts)

## Building and Running

### Quick Start
```bash
# Build all tests
make all

# Run only unit tests (safe, no dependencies)
make test_unit

# Run integration tests (requires Python3)
make test_integration

# Run all tests
make test
```

### Manual Building
```bash
# Unit tests only
c++ -Wall -Wextra -Werror -std=c++98 -I../include -I. -o test_cgi_unit \
    TestCGIUnit.cpp RequestExtended.cpp ../src/Response.cpp

# Integration tests
c++ -Wall -Wextra -Werror -std=c++98 -I../include -I. -o test_cgi \
    TestCGI.cpp RequestExtended.cpp ../src/Response.cpp ../src/CGI.cpp
```

## Test Output

The test framework provides colored output:
- 🔵 **BLUE** - Test starting
- 🟢 **GREEN** - Test passed  
- 🔴 **RED** - Test failed
- 📊 Final summary with pass/fail counts

Example output:
```
Running CGI Unit Test Suite
===========================================
[ RUN      ] test_split_fs_path_query
[Test] Split filesystem path and query string
[Expect] scriptPath='/tmp/script.py', query='name=test&value=123'
[Expect] scriptPath='/tmp/script.py', query=''
[Expect] scriptPath='', query=''
[       OK ] test_split_fs_path_query
...
ALL TESTS PASSED (27)
```

## Requirements

### For Unit Tests
- C++98 compatible compiler
- Basic POSIX environment

### For Integration Tests  
- Python3 installed and in PATH (`/usr/bin/env python3`)
- Bash shell for shell script tests
- Write access to `/tmp` directory
- `execve()`, `fork()`, `pipe()` system calls working

## Error Scenarios Tested

The test suite covers various error conditions:
- Invalid decision types
- Missing CGI scripts
- Malformed CGI output
- Pipe creation failures
- Fork failures
- Script execution failures

## Extending the Tests

To add new tests:

1. **Unit Tests**: Add to `TestCGIUnit.cpp` following the pattern:
```cpp
static void test_your_new_test() {
    std::cout << "[Test] Description" << std::endl;
    // Test code here
    ASSERT_EQ(expected, actual);
}
```

2. **Integration Tests**: Add to `TestCGI.cpp` with actual script creation:
```cpp
static void test_your_integration_test() {
    std::string scriptPath = "/tmp/test_script.py";
    createTestScript(scriptPath, "#!/usr/bin/env python3\nprint('Hello')");
    // Test code here
    cleanupTestFile(scriptPath);
}
```

3. **Add to main()**: Include your test in the `RUN_TEST()` calls

## Cleanup

The test suite automatically cleans up temporary files, but you can also run:
```bash
make clean
```

This removes test executables and any leftover test scripts in `/tmp`.
