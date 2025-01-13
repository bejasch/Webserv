

#pragma once

#ifndef ALLHEADERS_HPP
#define ALLHEADERS_HPP

#define MAX_HEADER_SIZE 8192	// 8 KB
#define MAX_HEADER_COUNT 100	// Arbitrary limit

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
#include <stdlib.h>
#include <netinet/in.h>
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


// #include "
#include "Config.hpp"
#include "HttpReq.hpp"
#include "HttpRes.hpp"
#include "Route.hpp"
#include "Server.hpp"
#include "ServerManager.hpp"

#endif
