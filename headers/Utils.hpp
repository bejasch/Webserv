#ifndef UTILS_HPP
#define UTILS_HPP

#include "AllHeaders.hpp"

int		stringToInt(const std::string &str);
bool	isStandaloneWord(const std::string& line, const std::string& word, size_t pos);
bool	isDirectory(const std::string &path);

#endif
