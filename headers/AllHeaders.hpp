

#pragma once

#ifndef ALLHEADERS_HPP
#define ALLHEADERS_HPP

class	HttpReq;
class	Route;

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <cstring>
#include <cstdlib> //probably illegal, used for malloc
#include <cstdio> //probably illegal, used for sprintf
#include <fstream>
#include <sstream>


// #include "
#include "Config.hpp"
#include "HttpReq.hpp"
#include "HttpRes.hpp"
#include "Route.hpp"
#include "Server.hpp"

#endif
