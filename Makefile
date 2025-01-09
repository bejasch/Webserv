
FLAGS = -std=c++98 -Wall -Wextra -Werror

test_requests:
	c++ src/request/HttpReq.cpp src/request/HttpReq_test.cpp -o test_requests $(FLAGS)
	./test_requests
	rm test_requests
