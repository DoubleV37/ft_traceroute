CC = gcc

NAME = ft_traceroute

SRC =	srcs/main.c \
		srcs/traceroute.c \
		srcs/traceroute_utils.c \
		srcs/parsing.c \
		srcs/commands.c \
		srcs/ping_utils.c

RM = rm -rf

MAKEFLAGS =	--no-print-directory

OBJ =	$(SRC:srcs/%.c=objs/%.o)

OBJ_DIR = objs

CFLAGS = -Wall -Wextra -Werror -g

all:	$(NAME)

objs/%.o: srcs/%.c
	$(CC) $(CFLAGS) -c $^ -o $(<:srcs/%.c=objs/%.o)

$(OBJ_DIR):
		mkdir $@

$(NAME):	$(OBJ_DIR) $(OBJ)
		@echo "**********************"
		@echo "*     \033[1;33mCOMPILING\033[1;0m      *"
		@echo "**********************\n"
		$(CC) $(CFLAGS) $(OBJ) -o $@
		@echo "**********************"
		@echo "*        \033[1;32mDONE\033[1;0m        *"
		@echo "**********************"

clean:
		$(RM) $(OBJ_DIR)
		@echo "**********************"
		@echo "*    \033[1;32mCLEANED OBJS\033[1;0m    *"
		@echo "**********************"

fclean:
		$(RM) $(OBJ_DIR) $(NAME)
		@echo "**********************"
		@echo "*     \033[1;32mCLEANED ALL\033[1;0m    *"
		@echo "**********************"

re:		fclean	all

.PHONY:	all clean fclean re
