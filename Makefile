CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98
NAME = ircserv

SRCS = main.cpp ConfigManager.cpp IrcMember.cpp Channel.cpp UnregisterMember.cpp\
       command/pass.cpp command/nick.cpp command/user.cpp command/privmsg.cpp \
       command/register.cpp command/quit.cpp command/PingPong.cpp command/join.cpp \
       command/Part.cpp command/kick.cpp command/mode.cpp command/invite.cpp \
       command/modeInvite.cpp command/modeKey.cpp command/modeOperator.cpp \
       command/modeLimit.cpp command/modeTopic.cpp command/Topic.cpp \
       util/util.cpp config/setup.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re