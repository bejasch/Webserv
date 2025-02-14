#ifndef UTILS_HPP
#define UTILS_HPP

#include "AllHeaders.hpp"

int			stringToInt(const std::string &str);
std::string	intToString(int value);
bool		isStandaloneWord(const std::string& line, const std::string& word, size_t pos);
std::string	trim(const std::string& str);
bool		isDirectory(const std::string &path);
bool		saveFile(const std::string &filename, const char* data, size_t size);
std::string getFileExtension(const std::string &target);
char		*cpp_strdup(const std::string str);
int			find_commented_line(std::string &line);
std::string removeTrailingSlash(std::string &path);
bool        fileExists(const char *filename);

std::vector<std::string> splitString(const std::string &str, const char delimiter);

// Guestbook functions
std::map<std::string, std::string>	parsePostData(const std::string &data);

void				saveGuestbookEntry(const std::string &name, const std::string &message);
const std::string	generateGuestbookHTML(const std::string &userName);

std::string resolvePath(const std::string &target, const std::string &route_path, const std::string &root_dir);

#endif
