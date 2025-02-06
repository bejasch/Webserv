
#pragma once
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "AllHeaders.hpp"

class Config {
	private:
		int				server_port;
		std::string		server_name;
		std::string		root_dir;
		int				max_body_size;
		std::string		default_file;

		std::map<int, std::string>	error_pages;
		std::vector<std::string>	allowed_methods;
		std::vector<Route *>		routes;

	public:
		Config();
		~Config();        

		// Setters
		void setPort(int port);
		void setName(std::string name);
		void setRootDirConfig(std::string root_dir);
		void setMaxBodySize(int max_body_size);
		void setDefaultFile(std::string default_file);
		void setErrorPage(int error_status, std::string error_page);
		void setAllowedMethods(const std::vector<std::string> &allowed_methods);
		void addRoute(Route *route);

		// Getters
		int			getPort() const;
		std::string	getName() const;
		std::string	getRootDirConfig() const;
		int			getMaxBodySize() const;
		std::string	getDefaultFile() const;
		const std::map<int, std::string>	&getErrorPages() const;

		Route	*getRouteForTarget(std::string &target);

		// Utils
		int		initialisedCheck();
		void	printConfig();
		void	freeConfig();
};

#endif
