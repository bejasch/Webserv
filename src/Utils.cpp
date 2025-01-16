#include "../headers/AllHeaders.hpp"

int stringToInt(const std::string &str) {
    return std::atoi(str.c_str());
}

bool isStandaloneWord(const std::string& line, const std::string& word, size_t pos) {
    // Check character before the word (if not at the start of the string)
    if (pos > 0 && std::isalnum(line[pos - 1])) {
        return false;
    }

    // Check character after the word (if not at the end of the string)
    size_t after = pos + word.length();
    if (after < line.size() && std::isalnum(line[after])) {
        return false;
    }

    return true;
}