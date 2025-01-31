

#pragma once

#ifndef ALLHEADERS_HPP
#define ALLHEADERS_HPP

#define MAX_HEADER_SIZE 8192	// 8 KB
#define MAX_HEADER_COUNT 100	// Arbitrary limit
#define MAX_BODY_SIZE 1048576	// 1 MB
#define MAX_RETRY_COUNT 5
#define GUESTBOOK_FILE "data/guestbook/guestbook.txt"

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
