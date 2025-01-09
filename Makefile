NAME = webserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRCS = src/main.cpp src/Config.cpp src/HttpReq.cpp src/HttpRes.cpp src/Route.cpp src/Server.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

test_requests:
	$(CXX) src/HttpReq.cpp testing/HttpReq_test.cpp -o test_requests $(CXXFLAGS)
	./test_requests
	rm test_requests


.PHONY: all clean fclean re
