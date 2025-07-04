NAME = ft_ping

SRCS = main.c dns.c ft_ping.c ft_ping_stat.c

OBJ_DIR = ./obj/

OBJS = $(addprefix $(OBJ_DIR), $(SRCS:.c=.o))

CFLAGS = -Wall -Werror -Wextra

all: $(NAME)

$(NAME): $(OBJS)
	gcc $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)%.o : %.c | $(OBJ_DIR)
	gcc $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all