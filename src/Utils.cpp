#include "../headers/AllHeaders.hpp"

int stringToInt(const std::string &str) {
	return std::atoi(str.c_str());
}

std::string	intToString(int value) {
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

// Helper function to trim leading and trailing whitespaces
std::string	trim(const std::string& str) {
	size_t start = str.find_first_not_of(" \t");
	if (start == std::string::npos)
		return ("");
	size_t end = str.find_last_not_of(" \t");
	return (str.substr(start, end - start + 1));
}

// Check if a path is a directory
bool	isDirectory(const std::string &path) {
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf) != 0)
		return (false);
	return (S_ISDIR(statbuf.st_mode));
}

// Function to save a file to disk
bool	saveFile(const std::string &filename, const char* data, size_t size) {
	std::ofstream file(filename.c_str(), std::ios::binary);
	if (!file.is_open())
		return (false);
	file.write(data, size);
	file.close();
	return (true);
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
const std::string	generateGuestbookHTML(const std::string &userName) {
	std::ostringstream html;

	html << "<html><head><title>Guestbook</title>";
	html << "<style>";
	html << "body { font-family: Arial, sans-serif; background-color:rgb(65, 157, 141); padding: 20px; }";
	html << "h1 { color: rgb(122, 29, 252); }"; // Header color orange
	html << "ul { list-style-type: disc; padding-left: 20px; }";
	html << "li { margin: 5px 0; }";
	html << "a { text-decoration: none; color: rgb(17, 0, 255); font-weight: bold; }";
	html << "a:hover { text-decoration: underline; }";
	html << "button { background-color: rgb(49, 146, 250); color: white; border: none; padding: 8px 12px;";
	html << "border-radius: 5px; cursor: pointer; font-size: 14px; }";
	html << "#login_status { position: fixed; top: 10px; right: 10px; color: white; padding: 12px 18px;";
	html << "border-radius: 20px; font-size: 16px; box-shadow: 3px 3px 12px rgba(0, 0, 0, 0.3); font-weight: bold; }";
	html << "</style></head><body>";

	// Login status box
	if (!userName.empty()) {
		html << "<div id='login_status' style='background-color: green;'>Logged in as " + userName + "</div>";
	} else {
		html << "<div id='login_status' style='background-color: red;'>Not logged in</div>";
	}
	html << "<button onclick=\"window.location.href='/index.html'\">Back to Main Page</button>";
	
	if (!userName.empty())
		html << "<h1>Hello " + userName + ", welcome to the Guestbook</h1>";
	else
		html << "<h1>Guestbook</h1>";

	html << "<form method='POST' action='/guestbook.html'>";
	html << "Name: <input type='text' name='name' id='nameField'><br>";
	html << "Message: <textarea name='message'></textarea><br>";
	html << "<input type='submit' name='action' value='Submit'>";
	html << "<input type='submit' name='action' value='Scramble.py'>";
	html << "<input type='submit' name='action' value='Capitalize.php'>";
	html << "</form><hr>";

	// Displaying existing messages
	html << "<h2>Messages</h2>";

	html << "<script>";				// JavaScript to autofill the name if userName is set
	html << "window.onload = function() {";
	html << "    var userName = '" + userName + "';";
	html << "    if (userName) {";
	html << "        document.getElementById('nameField').value = userName;";
	html << "    }";
	html << "};";
	html << "</script>";

	std::ifstream file(GUESTBOOK_FILE); 	// Load guestbook entries from file
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

std::vector<std::string> splitString(const std::string &str, const char delimiter) {
	std::vector<std::string> allowed_methods;
	std::string method;
	std::istringstream iss(str);
	while (std::getline(iss, method, delimiter)) {
		allowed_methods.push_back(method);
	}
	return (allowed_methods);
}

std::string getFileExtension(const std::string &target) {
	size_t dotPos = target.find_last_of('.');  // Find the last dot
	if (dotPos != std::string::npos) {
		return target.substr(dotPos);  // Extract the substring after the last dot
	}
	return "";  // No extension found
}
		
std::string resolvePath(const std::string &target, const std::string &route_path, const std::string &root_dir) {
	std::cout << "Target: " << target << " Route Path: " << route_path << " Root Dir: " << root_dir << std::endl;
	// Check if the target starts with route_path
	if (target.find(route_path) == 0) { // If route_path is a prefix of target
		if (target == route_path) {
			if (target.find_last_of('.') != std::string::npos) {
				return root_dir + target; // Keep full target if it's a file
			} else {
				return root_dir + "/"; // If it's a directory, return root_dir only
			}
		}
		std::string resolvedPath = target.substr(route_path.length());

		if (resolvedPath.empty() || resolvedPath[0] != '/')
			resolvedPath = "/" + resolvedPath;
		return (root_dir + resolvedPath);
	}
	return ""; // Return empty string if route_path is not a prefix of target
}

char	*cpp_strdup(const std::string str) {
	char *dup;

	dup = new char[str.size() + 1];
	if (dup == NULL) {
		std::cerr << "Failed to allocate memory: " << std::strerror(errno) << std::endl;
		return NULL;
	}
	std::copy(str.begin(), str.end(), dup);
	dup[str.size()] = '\0';
	return (dup);
}

int	find_commented_line(std::string &line) {
	int i = 0;
	while (line[i] == ' ' || line[i] == '\t')
		i++;
	if (line[i] == '#')
		return (1);
	return (0);
}

std::string	removeTrailingSlash(std::string &path) {
	if (path == "/")
		return path;
	if (path[path.length() - 1] == '/')
		path = path.substr(0, path.length() - 1);
	return path;
}
