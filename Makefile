# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: lenygarcia <lenygarcia@student.42.fr>      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/08/25 11:17:35 by lengarci          #+#    #+#              #
#    Updated: 2025/08/29 14:55:34 by lenygarcia       ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

.SILENT:

NAME		=	webserv
CC			=	g++
CFLAGS		=	-Wall -Wextra -Werror -std=c++98 -g3
RM			=	rm -f
DIR_OBJ		=	.obj/
SRCS		=	main.cpp \
				src/Server.cpp src/ConfigParse.cpp
OBJS		=	$(addprefix $(DIR_OBJ), $(SRCS:.cpp=.o))
HEADER		=	include/Server.hpp include/ConfigParse.hpp

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
		@printf "\033[0;32mObject files removed\033[0m\n"

fclean:		clean
		$(RM) $(NAME)
		@printf "\033[0;32m$(NAME) removed\033[0m\n"

re:	fclean
		@make all --silent

testconfig:
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

.PHONY:		all clean fclean re
