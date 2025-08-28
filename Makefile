# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: macauchy <macauchy@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/08/25 11:17:35 by lengarci          #+#    #+#              #
#    Updated: 2025/08/28 15:37:37 by macauchy         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

.SILENT:

NAME		=	webserv
CC			=	g++
CFLAGS		=	-Wall -Wextra -Werror -std=c++98
RM			=	rm -f
DIR_OBJ		=	.obj/
SRCS		=	main.cpp \
				src/Server.cpp \
				src/Request.cpp \
				src/Parser.cpp	\
				src/Response.cpp
OBJS		=	$(addprefix $(DIR_OBJ), $(SRCS:.cpp=.o))
HEADER		=	include/Server.hpp \
				include/Request.hpp \
				include/Parser.hpp \
				include/Response.hpp

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
		@printf "\033[0;32m$(NAME) removed\033[0m\n"

re:	fclean
		@make all --silent

# Test targets (separate binaries to avoid multiple mains)
PARSER_TEST_BIN = tests/run_parser_tests
REQUEST_TEST_BIN = tests/run_request_tests
PARSER_TEST_SRC = tests/TestParser.cpp
REQUEST_TEST_SRC = tests/TestRequest.cpp

$(PARSER_TEST_BIN): $(PARSER_TEST_SRC) src/Parser.cpp src/Request.cpp tests/test_harness.hpp
	@mkdir -p tests
	$(CC) $(CFLAGS) -I./include -o $(PARSER_TEST_BIN) $(PARSER_TEST_SRC) src/Parser.cpp src/Request.cpp
	@printf "\033[0;32mTests built: %s\033[0m\n" $(PARSER_TEST_BIN)

$(REQUEST_TEST_BIN): $(REQUEST_TEST_SRC) src/Request.cpp tests/test_harness.hpp
	@mkdir -p tests
	$(CC) $(CFLAGS) -I./include -o $(REQUEST_TEST_BIN) $(REQUEST_TEST_SRC) src/Request.cpp
	@printf "\033[0;32mTests built: %s\033[0m\n" $(REQUEST_TEST_BIN)

.PHONY:		all clean fclean re test
test: $(PARSER_TEST_BIN) $(REQUEST_TEST_BIN)
	@printf "\033[0;34mRunning parser tests:\033[0m\n"
	./$(PARSER_TEST_BIN)
	@printf "\033[0;34mRunning request tests:\033[0m\n"
	./$(REQUEST_TEST_BIN)
