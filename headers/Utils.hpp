#ifndef UTILS_HPP
#define UTILS_HPP

#include "AllHeaders.hpp"

int		    stringToInt(const std::string &str);
std::string	intToString(int value);
bool	    isStandaloneWord(const std::string& line, const std::string& word, size_t pos);
bool	    isDirectory(const std::string &path);
std::vector<std::string> splitString(const std::string &str, const char delimiter);

// Guestbook functions
std::map<std::string, std::string>	parsePostData(const std::string &data);

void				saveGuestbookEntry(const std::string &name, const std::string &message);
const std::string	generateGuestbookHTML(void);

bool	deleteFileDir(const std::string &path);

#endif
