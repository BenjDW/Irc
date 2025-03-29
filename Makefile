NAME = ircserv

INCLUDE = ./

CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98

FILES = src/main.cpp src/serv.cpp src/client.cpp src/channel.cpp src/command.cpp
OBJECTS = $(FILES:.cpp=.o)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INCLUDE)

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ -I $(INCLUDE)

clean:
	rm -rf $(OBJECTS)


fclean: clean
	rm -rf $(NAME)

re: fclean all