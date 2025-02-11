#ifndef ROUTE_HPP
#define ROUTE_HPP

#include "AllHeaders.hpp"

class Route {
	private:
		std::string				 	path;
		std::vector<std::string>	allowed_methods;
		std::string				 	root_dir;
		std::string				 	index_file;
		bool						autoindex;
		int						 	redirect_status;
		std::string				 	redirect_url;
		bool						autoindex_set;

	public:
		Route();
		Route(const Route &other);
		Route operator=(const Route &another);
		~Route();
		
		//setter functions
		void		setPath(const std::string &path);
		void		setAllowedMethods(const std::vector<std::string> &allowed_methods);
		void		setRootDirRoute(const std::string &root_dir);
		void		setIndexFile(const std::string &index_file);
		void		setAutoindex(bool autoindex);
		void		setRedirectStatus(int redirect_status);
		void		setRedirectUrl(const std::string &redirect_url);

		//getter functions
		std::string					getPath() const { return path; }
		std::vector<std::string>	getAllowedMethods() const { return allowed_methods; }
		std::string					getRootDirRoute() const { return root_dir; }
		std::string					getIndexFile() const { return index_file; }
		bool						getAutoindex() const { return autoindex; }
		int							getRedirectStatus() const { return redirect_status; }
		std::string					getRedirectUrl() const { return redirect_url; }
		bool						getAutoindexSet() const { return autoindex_set; }

		bool	containsTarget(const std::string &target) const;
		bool	allowsMethod(const std::string &method) const;
		int		checkRoute(Route *route);
		void	cleanRoute(Route *route);

		//utils
		void printRoute();
};

#endif
