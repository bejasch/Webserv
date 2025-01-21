#ifndef UTILS_HPP
#define UTILS_HPP

#include "AllHeaders.hpp"

int		stringToInt(const std::string &str);
bool	isStandaloneWord(const std::string& line, const std::string& word, size_t pos);
bool	isDirectory(const std::string &path);

// Guestbook functions
std::map<std::string, std::string>	parsePostData(const std::string &data);

void				saveGuestbookEntry(const std::string &name, const std::string &message);
const std::string	generateGuestbookHTML(void);

#endif
