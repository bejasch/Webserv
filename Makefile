
NAME = webserv
SRC_DIR		= src
OBJ_DIR		= obj
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRCS = src/main.cpp src/Config.cpp src/HttpReq.cpp src/HttpRes.cpp src/Route.cpp src/Server.cpp
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	cc $(FLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

# Testing - will be deleted later
test_requests:
	$(CXX) src/HttpReq.cpp testing/HttpReq_test.cpp -o test_requests $(CXXFLAGS)
	./test_requests
	rm test_requests


.PHONY: all clean fclean re
