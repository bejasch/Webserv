#include "../headers/AllHeaders.hpp"

int stringToInt(const std::string &str) {
    return std::atoi(str.c_str());
}

std::string intToString(int value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
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

// Check if a path is a directory
bool	isDirectory(const std::string &path) {
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf) != 0)
		return (false);
	return (S_ISDIR(statbuf.st_mode));
}


// Parse HTTP POST data (application/x-www-form-urlencoded)
// Example body: "name=Ben&message=This+is+a+test%21"
std::map<std::string, std::string>	parsePostData(const std::string &data) {
	std::map<std::string, std::string> postData;
	std::istringstream	stream(data);
	std::string			pair;

	while (std::getline(stream, pair, '&')) {
		size_t pos = pair.find('=');
		if (pos != std::string::npos) {
			std::string key = pair.substr(0, pos);
			std::string value = pair.substr(pos + 1);
			// Decode URL-encoded values (basic implementation)
			std::replace(value.begin(), value.end(), '+', ' ');
			// Decode URL-encoded values (extended implementation)
			for (size_t i = 0; i < value.length(); ++i) {
				if (value[i] == '%') {
					char c;
					if (sscanf(value.substr(i + 1, 2).c_str(), "%2hhx", &c) == 1) {
						value[i] = c;
						value.erase(i + 1, 2);
					}
				}
			}
			postData[key] = value;
		}
	}
	return (postData);
}

// Save a new entry to the file located at GUESTBOOK_FILE
void	saveGuestbookEntry(const std::string &name, const std::string &message) {
    if (name.empty() || message.empty()) {
		return;
	}
	if (name.find('|') != std::string::npos || message.find('|') != std::string::npos) {
		std::cerr << "Warning: Invalid characters in guestbook entry.\n";
		return;
	}
	if (name.length() > 100 || message.length() > 1000) {
		std::cerr << "Warning: Guestbook entry too long.\n";
		return;
	}

	std::ofstream file(GUESTBOOK_FILE, std::ios::app); // Append mode
	if (file) {
		file << name << "|" << message << "\n";				// Append entry to file	
	} else {
		std::cerr << "Error: Could not open file " << GUESTBOOK_FILE << " for writing.\n";
	}
}

// Generate HTML for guestbook
const std::string	generateGuestbookHTML(void) {
    std::ostringstream html;
    html << "<!DOCTYPE html><html><head><title>Guestbook</title></head><body>";
    html << "<h1>Welcome to the Guestbook</h1>";
    html << "<form method='POST' action='/guestbook.html'>"
         << "Name: <input type='text' name='name'><br>"
         << "Message: <textarea name='message'></textarea><br>"
         << "<button type='submit'>Submit</button></form><hr>";
    html << "<h2>Messages</h2>";

	// Load guestbook entries from file
	std::ifstream file(GUESTBOOK_FILE);
	if (file) {
		std::string line;
		while (std::getline(file, line)) {
			size_t sep = line.find('|');
			if (sep != std::string::npos) {
				html << "<p><strong>" << line.substr(0, sep) << ":</strong> " << line.substr(sep + 1) << "</p>";
			}
		}
		file.close();
	}
    html << "</body></html>";
	return (html.str());
}

bool	deleteFileDir(const std::string &path) {
	pid_t	pid = fork();
	if (pid < 0) {
		perror("Fork failed");
		return (false);
	}

	if (pid == 0) {
		const char *const args[] = {"/bin/rm", "-rf", path.c_str(), NULL};
		if (execve("/bin/rm", (char *const *)args, NULL) == -1) {
            // If execve fails, report the error
            perror("execve failed");
            return (false);  // Return false if execve fails
        }
	} else {
		// Parent process: Wait for the child process to finish
		int status;
		waitpid(pid, &status, 0);

		// Check the exit status of the child process
		if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
			std::cout << "Successful DELETE with target: " << path << std::endl;
			return (true);
		} else {
			std::cerr << "Failed to delete from request with target: " << path << std::endl;
			return (false);
		}
	}
	return (false);
}
