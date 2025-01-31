#include "../headers/AllHeaders.hpp"

CGI::CGI(Route *route) {
    // this->cgiPath = route->getCGIPath();
    this->pid = 0;
    this->route = route;
    std::cout << "CGI default constructor called" << std::endl;
}

CGI::~CGI() {
    std::cout << "CGI destructor called" << std::endl;
}

void CGI::setAllEnv(HttpReq &httpRequest, Server &server) {
    env["SERVER_NAME"] = server.getConfig()->getName();
    env["SERVER_PORT"] = intToString(server.getConfig()->getPort());
    env["SERVER_PROTOCOL"] = httpRequest.getProtocol();
    env["REQUEST_METHOD"] = httpRequest.getMethod();
    env["SCRIPT_NAME"] = httpRequest.getTarget();
    env["CONTENT_TYPE"] = route->getPath(); //.py
}

std::string CGI::executeCGI(HttpReq &httpRequest, Server &server) {
    setAllEnv(httpRequest, server);
    std::string scriptPath = "data/cgi-bin" + env["SCRIPT_NAME"];
    std::cout << "Executing CGI script: " << scriptPath << std::endl;

    printf("Current working directory: %s\n", getcwd(NULL, 0));

    // Create a pipe to communicate with the child process
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("Failed to create pipe");
        return ("404");
    }

    pid = fork();
    if (pid == -1) {
        perror("Failed to fork");
        return ("404");
    }

    if (pid == 0) {
        // Child process
        close(pipe_fd[0]); // Close the read end of the pipe

        // Redirect stdout and stderr to the write end of the pipe
        dup2(pipe_fd[1], STDOUT_FILENO);
        dup2(pipe_fd[1], STDERR_FILENO);
        close(pipe_fd[1]); // Close the write end of the pipe

        // Convert the env map to a char* array
        char *envp[env.size() + 1];
        int i = 0;
        for (std::map<std::string, std::string>::iterator it = env.begin(); it != env.end(); ++it) {
            envp[i] = strdup((it->first + "=" + it->second).c_str());
            ++i;
        }
        envp[i] = NULL;

        //Prepare the arguments (argv)
        //TODO: handle args
        const char *argv[] = {scriptPath.c_str(), NULL}; // No additional arguments

        // Execute the script using execve
        if (execve(scriptPath.c_str(), const_cast<char *const *>(argv), envp) == -1) {
            perror("Failed to execute script");
            exit(EXIT_FAILURE); // Exit child process if execve fails
        }
    } else {
        // Parent process
        close(pipe_fd[1]); // Close the write end of the pipe

        // Read the output from the child process
        char buffer[4096];
        int bytes_read;
        std::string output;
        while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer))) > 0) {
            output.append(buffer, bytes_read);
        }
        close(pipe_fd[0]); // Close the read end of the pipe

        // Wait for the child process to finish
        int status;
        waitpid(pid, &status, 0);

        // Check the exit status of the child process
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            std::cout << "Successful CGI execution" << std::endl;
            std::cout << "Output: " << output << std::endl;
            return (output);
        } else {
            std::cerr << "Failed to execute CGI. Exit status: " << WEXITSTATUS(status) << std::endl;
            std::cerr << "Output (including errors): " << output << std::endl;
            return ("404");
        }
    }
    return ("404");
}

void CGI::printCGI() {
    std::cout << "CGI path: " << route->getPath() << std::endl;
    std::cout << "CGI env:" << std::endl;
    for (std::map<std::string, std::string>::iterator it = env.begin(); it != env.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }
}
