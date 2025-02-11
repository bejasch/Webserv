#ifndef ALLHEADERS_HPP
#define ALLHEADERS_HPP

#define MAX_HEADER_SIZE		8192	// 8 KB
#define MAX_HEADER_COUNT	100		// Arbitrary limit
#define MAX_RETRY_COUNT		5		// Maximum number of retries for write failures
#define MAX_EVENTS			100		// Max events for epoll_wait
#define GUESTBOOK_FILE		"data/guestbook/guestbook.txt"

#define RESET   "\033[0m"      // Reset all colors
#define RED     "\033[31m"     // Red text
#define GREEN   "\033[32m"     // Green text
#define YELLOW  "\033[33m"     // Yellow text
#define BLUE    "\033[34m"     // Blue text
#define CYAN    "\033[36m"     // Cyan text

class	CGI;
class	Config;
class	HttpReq;
class	HttpRes;
class	Route;
class	Server;
class	ServerManager;

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <dirent.h>
#include <cstring>
#include <cstdlib> //probably illegal, used for malloc
#include <cstdio> //probably illegal, used for sprintf
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <csignal>
#include <cstdlib>  // For strtoul
#include <ctime>

// #include "
#include "Config.hpp"
#include "HttpReq.hpp"
#include "HttpRes.hpp"
#include "Route.hpp"
#include "Server.hpp"
#include "ServerManager.hpp"
#include "Utils.hpp"
#include "CGI.hpp"

#endif
