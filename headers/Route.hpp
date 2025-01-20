#pragma once
#ifndef ROUTE_HPP
#define ROUTE_HPP

#include "AllHeaders.hpp"

class Route {
    private:
        std::string                 path;
        std::vector<std::string>    allowed_methods; //{"GET", "POST", "DELETE", "PUT"};
        std::string                 root_dir;
        std::string                 index_file;
        bool                        autoindex;
        int                         redirect_status;
        std::string                 redirect_url;

    public:
        Route();
        ~Route();
        
        //setter functions
        void setPath(const std::string &path);
        void setAllowedMethods(const std::vector<std::string> &allowed_methods);
        void setRootDir(const std::string &root_dir);
        void setIndexFile(const std::string &index_file);
        void setAutoindex(std::string autoindex);
        void setRedirectStatus(int redirect_status);
        void setRedirectUrl(const std::string &redirect_url);

        //getter functions
        std::string getPath() const { return path; }
        std::vector<std::string> getAllowedMethods() const { return allowed_methods; }
        std::string getRootDir() const { return root_dir; }
        std::string getIndexFile() const { return index_file; }
        bool getAutoindex() const { return autoindex; }
        int getRedirectStatus() const { return redirect_status; }
        std::string getRedirectUrl() const { return redirect_url; }

		bool	containsTarget(const std::string &target) const;
		bool	allowsMethod(const std::string &method) const;

        //utils
        void printRoute();
};

#endif
