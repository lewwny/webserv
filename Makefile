# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: macauchy <macauchy@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/08/25 11:17:35 by lengarci          #+#    #+#              #
#    Updated: 2025/09/05 16:31:18 by macauchy         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

.SILENT:

NAME		=	webserv
CC			=	g++
CFLAGS		=	-Wall -Wextra -Werror -std=c++98 -g3
RM			=	rm -f
DIR_OBJ		=	.obj/
SRCS		=	main.cpp \
				src/Server.cpp \
				src/Request.cpp \
				src/Parser.cpp	\
				src/Response.cpp \
				src/Router.cpp \
				src/ConfigParse.cpp \
				src/ServerManager.cpp \
				src/Config.cpp
OBJS		=	$(addprefix $(DIR_OBJ), $(SRCS:.cpp=.o))
HEADER		=	include/Server.hpp \
				include/Request.hpp \
				include/Parser.hpp \
				include/Response.hpp \
				include/Router.hpp \
				include/ConfigParse.hpp \
				include/ServerManager.hpp \
				include/Config.hpp

all:		$(NAME)

$(NAME):	$(OBJS)
		$(CC) $(CFLAGS) -o $@ $^
		@printf "\033[0;32m$(NAME) created\033[0m\n"

$(DIR_OBJ)%.o:	%.cpp $(HEADER)
		@mkdir -p $(dir $@)
		$(CC) $(CFLAGS) -c $< -o $@
		@printf "\033[0;34mCompiled:\033[0m $<\n"

clean:
		$(RM) -r $(DIR_OBJ)
		$(RM) 
		@printf "\033[0;32mObject files removed\033[0m\n"

fclean:		clean
		$(RM) $(NAME)
		$(RM) $(PARSER_TEST_BIN) $(REQUEST_TEST_BIN) $(RESPONSE_TEST_BIN) $(ROUTER_TEST_BIN) $(INTEGRATION_TEST_BIN)
		@printf "\033[0;32m$(NAME) removed\033[0m\n"

re:	fclean
		@make all --silent

# Test targets (separate binaries to avoid multiple mains)
PARSER_TEST_BIN = tests/run_parser_tests
REQUEST_TEST_BIN = tests/run_request_tests
RESPONSE_TEST_BIN = tests/run_response_tests
ROUTER_TEST_BIN = tests/run_router_tests
INTEGRATION_TEST_BIN = tests/run_integration_tests

PARSER_TEST_SRC = tests/TestParser.cpp
REQUEST_TEST_SRC = tests/TestRequest.cpp
RESPONSE_TEST_SRC = tests/TestResponse.cpp
ROUTER_TEST_SRC = tests/TestRouterSimple.cpp
INTEGRATION_TEST_SRC = tests/TestIntegrationSimple.cpp

$(PARSER_TEST_BIN): $(PARSER_TEST_SRC) src/Parser.cpp src/Request.cpp tests/test_harness.hpp
	@mkdir -p tests
	$(CC) $(CFLAGS) -I./include -o $(PARSER_TEST_BIN) $(PARSER_TEST_SRC) src/Parser.cpp src/Request.cpp
	@printf "\033[0;32mTests built: %s\033[0m\n" $(PARSER_TEST_BIN)

$(REQUEST_TEST_BIN): $(REQUEST_TEST_SRC) src/Request.cpp tests/test_harness.hpp
	@mkdir -p tests
	$(CC) $(CFLAGS) -I./include -o $(REQUEST_TEST_BIN) $(REQUEST_TEST_SRC) src/Request.cpp
	@printf "\033[0;32mTests built: %s\033[0m\n" $(REQUEST_TEST_BIN)

$(RESPONSE_TEST_BIN): $(RESPONSE_TEST_SRC) src/Response.cpp tests/test_harness.hpp
	@mkdir -p tests
	$(CC) $(CFLAGS) -I./include -o $(RESPONSE_TEST_BIN) $(RESPONSE_TEST_SRC) src/Response.cpp
	@printf "\033[0;32mTests built: %s\033[0m\n" $(RESPONSE_TEST_BIN)

$(ROUTER_TEST_BIN): $(ROUTER_TEST_SRC) src/Request.cpp tests/test_harness.hpp
	@mkdir -p tests
	$(CC) $(CFLAGS) -I./include -o $(ROUTER_TEST_BIN) $(ROUTER_TEST_SRC) src/Request.cpp
	@printf "\033[0;32mTests built: %s\033[0m\n" $(ROUTER_TEST_BIN)

$(INTEGRATION_TEST_BIN): $(INTEGRATION_TEST_SRC) src/Parser.cpp src/Request.cpp src/Response.cpp tests/test_harness.hpp
	@mkdir -p tests
	$(CC) $(CFLAGS) -I./include -o $(INTEGRATION_TEST_BIN) $(INTEGRATION_TEST_SRC) src/Parser.cpp src/Request.cpp src/Response.cpp
	@printf "\033[0;32mTests built: %s\033[0m\n" $(INTEGRATION_TEST_BIN)

testconfig: all
		@printf "\033[0;33mTesting empty file:\033[0m\n"
		@-./webserv conf/bad/empty.conf
		@printf "\033[0;33mTesting bad extension:\033[0m\n"
		@-./webserv conf/bad/bad_extension.txt
		@printf "\033[0;33mTesting bad permissions:\033[0m\n"
		@-./webserv conf/bad/bad_permissions.conf
		@printf "\033[0;33mTesting no RBRACE:\033[0m\n"
		@-./webserv conf/bad/no_rbrace.conf
		@-./webserv conf/bad/no_rbrace2.conf
		@printf "\033[0;33mTesting no port:\033[0m\n"
		@-./webserv conf/bad/no_port.conf
		@printf "\033[0;33mTesting bad port:\033[0m\n"
		@-./webserv conf/bad/bad_port.conf
		@printf "\033[0;33mTesting double port:\033[0m\n"
		@-./webserv conf/bad/double_port.conf
		@printf "\033[0;33mTesting double path:\033[0m\n"
		@-./webserv conf/bad/double_path.conf
		@printf "\033[0;33mTesting bad autoindex:\033[0m\n"
		@-./webserv conf/bad/bad_autoindex.conf
		@printf "\033[0;33mTesting bad errorpage:\033[0m\n"
		@-./webserv conf/bad/bad_errorpage.conf

.PHONY:		all clean fclean re test testconfig test-verbose test-quiet
test: $(PARSER_TEST_BIN) $(REQUEST_TEST_BIN) $(RESPONSE_TEST_BIN) $(ROUTER_TEST_BIN) $(INTEGRATION_TEST_BIN)
	@printf "\033[0;34mRunning parser tests:\033[0m\n"
	./$(PARSER_TEST_BIN)
	@printf "\033[0;34mRunning request tests:\033[0m\n"
	./$(REQUEST_TEST_BIN)
	@printf "\033[0;34mRunning response tests:\033[0m\n"
	./$(RESPONSE_TEST_BIN)
	@printf "\033[0;34mRunning router tests:\033[0m\n"
	./$(ROUTER_TEST_BIN)
	@printf "\033[0;34mRunning integration tests:\033[0m\n"
	./$(INTEGRATION_TEST_BIN)

test-verbose: $(PARSER_TEST_BIN) $(REQUEST_TEST_BIN) $(RESPONSE_TEST_BIN) $(ROUTER_TEST_BIN) $(INTEGRATION_TEST_BIN)
	@printf "\033[0;34mRunning parser tests (verbose):\033[0m\n"
	./$(PARSER_TEST_BIN) --verbose
	@printf "\033[0;34mRunning request tests (verbose):\033[0m\n"
	./$(REQUEST_TEST_BIN) --verbose
	@printf "\033[0;34mRunning response tests (verbose):\033[0m\n"
	./$(RESPONSE_TEST_BIN) --verbose
	@printf "\033[0;34mRunning router tests (verbose):\033[0m\n"
	./$(ROUTER_TEST_BIN) --verbose
	@printf "\033[0;34mRunning integration tests (verbose):\033[0m\n"
	./$(INTEGRATION_TEST_BIN) --verbose

test-quiet: $(PARSER_TEST_BIN) $(REQUEST_TEST_BIN) $(RESPONSE_TEST_BIN) $(ROUTER_TEST_BIN) $(INTEGRATION_TEST_BIN)
	@printf "\033[0;34mRunning all tests (quiet mode):\033[0m\n"
	./$(PARSER_TEST_BIN) --quiet
	./$(REQUEST_TEST_BIN) --quiet
	./$(RESPONSE_TEST_BIN) --quiet
	./$(ROUTER_TEST_BIN) --quiet
	./$(INTEGRATION_TEST_BIN) --quiet

help-test:
	@printf "\033[0;33mTest targets:\033[0m\n"
	@printf "  make test         - Run all tests with normal verbosity\n"
	@printf "  make test-quiet   - Run all tests with minimal output\n"
	@printf "  make test-verbose - Run all tests with detailed output\n"
	@printf "\033[0;33mIndividual test options:\033[0m\n"
	@printf "  ./tests/run_*_tests --quiet    - Run specific test in quiet mode\n"
	@printf "  ./tests/run_*_tests --verbose  - Run specific test in verbose mode\n"
	@printf "  ./tests/run_*_tests --help     - Show test help\n"
