
NAME = webserv
SRC_DIR		= src
OBJ_DIR		= obj
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -g -std=c++98

SRCS = src/main.cpp src/Config.cpp src/HttpReq.cpp src/HttpRes.cpp src/Route.cpp src/Server.cpp src/ServerManager.cpp src/Utils.cpp src/CGI.cpp
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

all: $(NAME)
	@echo "🚀 Webserv project compiled successfully. You are good to go."

$(NAME): $(OBJS)
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
