#ifndef UTILS_HPP
#define UTILS_HPP

#include "AllHeaders.hpp"

int		    stringToInt(const std::string &str);
std::string	intToString(int value);
bool	    isStandaloneWord(const std::string& line, const std::string& word, size_t pos);
std::string	trim(const std::string& str);
bool	    isDirectory(const std::string &path);
bool		saveFile(const std::string &filename, const char* data, size_t size);
std::string getFileExtension(const std::string &target);

std::vector<std::string> splitString(const std::string &str, const char delimiter);

// Guestbook functions
std::map<std::string, std::string>	parsePostData(const std::string &data);

void				saveGuestbookEntry(const std::string &name, const std::string &message);
const std::string	generateGuestbookHTML(const std::string &userName);

bool	deleteFileDir(const std::string &path);

#endif
