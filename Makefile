
FLAGS = -std=c++98 -Wall -Wextra -Werror

test_requests:
	c++ src/requests/HttpReq.cpp src/requests/HttpReq_test.cpp -o test_requests $(FLAGS)
	./test_requests
	rm test_requests
