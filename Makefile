# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: lengarci <lengarci@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/08/25 11:17:35 by lengarci          #+#    #+#              #
#    Updated: 2025/08/25 11:48:03 by lengarci         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

.SILENT:

NAME		=	webserv
CC			=	g++
CFLAGS		=	-Wall -Wextra -Werror -std=c++98
RM			=	rm -f
DIR_OBJ		=	.obj/
SRCS		=	main.cpp \
				src/Server.cpp 
OBJS		=	$(addprefix $(DIR_OBJ), $(SRCS:.cpp=.o))
HEADER		=	include/Server.hpp

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

.PHONY:		all clean fclean re