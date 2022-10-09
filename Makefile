NAME=woody-woodpacker
CC=clang
CFLAGS=-Wall -Wextra -Werror

SRCS_FILES=main.c get_file.c check_file.c woody.c
OBJS_FILES=$(SRCS_FILES:.c=.o)

SRCS_DIR=srcs/
OBJS_DIR=objs/
INC_DIR=includes

SRCS= $(addprefix $(SRCS_DIR), $(SRCS_FILES))
OBJS= $(addprefix $(OBJS_DIR), $(OBJS_FILES))

.PHONY: all clean fclean re

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS) -I$(INC_DIR)

$(OBJS_DIR)%.o: $(SRCS_DIR)%.c
	@mkdir $(OBJS_DIR) 2>&- || true
	$(CC) $(CFLAGS) -I$(INC_DIR) -o $@ -c $<

clean:
	rm -f $(OBJS)
	rm -f payload.o

fclean: clean
	rm -rf $(OBJS_DIR)
	rm -f $(NAME)
	rm -f payload
	rm -f test

re: fclean all

test:
	$(CC) -o test test.c

payload: payload.s
	nasm -f elf64 -o payload.o payload.s && ld -o payload payload.o
