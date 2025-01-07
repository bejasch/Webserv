
#include "HttpReq.hpp"
#include <iostream>

HttpReq::HttpReq() {}
HttpReq::~HttpReq() {}

int	HttpReq::parse(std::string &buffer) {
	int httpStatus = 200;	// OK
	try
	{
		if ((httpStatus = parseStartLine(buffer)) != 200)
			return httpStatus;
		if ((httpStatus = parseHeaders(buffer)) != 200)
			return httpStatus;
		if ((httpStatus = parseSpecialHeaders(buffer)) != 200)
			return httpStatus;
		
		if ((httpStatus = parseBody(buffer)) != 200)
			return httpStatus;
		
		if (buffer_section_ == COMPLETE || httpStatus == 100)
		buffer_section_ = COMPLETE;
		else if (buffer_section_ == ERROR || (httpStatus != 200 && httpStatus != 100))
		buffer_section_ = ERROR;
	}
	catch (const std::exception &e)
	{
		std::cerr << "Parsing-exception: " << e.what() << std::endl;
		return 500;
	}
}

int	HttpReq::parseStartLine(std::string &buffer) {
	std::string::size_type pos = 0;
	std::string::size_type prev = 0;

	if ((pos = buffer.find(" ")) != std::string::npos) {
		method = buffer.substr(prev, pos - prev);
		prev = pos + 1;
	}
	if ((pos = buffer.find(" ", prev)) != std::string::npos) {
		target = buffer.substr(prev, pos - prev);
		prev = pos + 1;
	}
	protocol = buffer.substr(prev);
	return 0;
}

{
	std::string line;
	std::string::size_type pos = 0;
	std::string::size_type prev = 0;

	size_t endOfStartLine = buffer.find("\r\n");
	if (endOfStartLine == std::string::npos)
		return 400;

	while ((pos = buffer.find("\r\n", prev)) != std::string::npos) {
		line = buffer.substr(prev, pos - prev);
		if (line.empty()) {
			break;
		}
		if (prev == 0) {
			parseStartLine(line);
		} else {
			parseHeaders(line);
		}
		prev = pos + 2;
	}
	parseBody(buffer.substr(prev));
	return 0;
}
