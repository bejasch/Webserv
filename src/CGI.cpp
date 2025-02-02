#include "../headers/AllHeaders.hpp"

CGI::CGI() : pid(0), env({}) {
    std::cout << "CGI default constructor called" << std::endl;
}

CGI::~CGI() {
    std::cout << "CGI destructor called" << std::endl;
}

void CGI::setAllEnv(HttpReq &httpRequest) {
    env["SERVER_PROTOCOL"] = httpRequest.getProtocol();
    env["REQUEST_METHOD"] = httpRequest.getMethod();
    env["SCRIPT_NAME"] = httpRequest.getTarget();
}

std::string CGI::executeCGI_GET(HttpReq &httpRequest) {
    setAllEnv(httpRequest);
    std::string scriptPath = "data/cgi-bin" + env["SCRIPT_NAME"];
    std::cout << "Executing CGI script (GET): " << scriptPath << std::endl;

    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("Failed to create pipe");
        return "500";  // Internal Server Error
    }

    pid = fork();
    if (pid == -1) {
        perror("Failed to fork");
        return "500";
    }

    if (pid == 0) {  // Child process
        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);
        dup2(pipe_fd[1], STDERR_FILENO);
        close(pipe_fd[1]);

        char *envp[env.size() + 1];
        int i = 0;
        for (auto it = env.begin(); it != env.end(); ++it) {
            envp[i] = strdup((it->first + "=" + it->second).c_str());
            ++i;
        }
        envp[i] = NULL;

        const char *argv[] = {scriptPath.c_str(), NULL}; // No args
        execve(scriptPath.c_str(), const_cast<char *const *>(argv), envp);
        perror("Failed to execute script");
        exit(EXIT_FAILURE);
    } 

    close(pipe_fd[1]);
    char buffer[4096];
    std::string output;
    int bytes_read;
    while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer))) > 0) {
        output.append(buffer, bytes_read);
    }
    close(pipe_fd[0]);

    int status;
    waitpid(pid, &status, 0);

    return (WIFEXITED(status) && WEXITSTATUS(status) == 0) ? output : "500";
}

std::string CGI::executeCGI_POST(HttpReq &httpRequest, const std::map<std::string, std::string> &formData) {
    setAllEnv(httpRequest);

    std::string scriptPath = "data/cgi-bin/modify_comments.py";
    std::cout << "Executing CGI script (POST): " << scriptPath << std::endl;

    int pipe_fd[2], stdin_pipe[2];
    if (pipe(pipe_fd) == -1 || pipe(stdin_pipe) == -1) {
        perror("Failed to create pipes");
        return "500";
    }

    pid = fork();
    if (pid == -1) {
        perror("Failed to fork");
        return "500";
    }

    if (pid == 0) {  // Child process
        close(pipe_fd[0]);
        close(stdin_pipe[1]);

        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(pipe_fd[1], STDOUT_FILENO);
        
        close(stdin_pipe[0]);
        close(pipe_fd[1]);

        char *const args[] = {(char *)scriptPath.c_str(), NULL};
        execve(scriptPath.c_str(), args, NULL);
        perror("Failed to execute script");
        exit(EXIT_FAILURE);
    }

    // Parent process
    close(pipe_fd[1]);
    close(stdin_pipe[0]);

    // Write form data to the script's stdin
    std::string postData;
    for (const auto &pair : formData) {
        if (!postData.empty()) postData += "&";
        postData += pair.first + "=" + pair.second;
    }
    write(stdin_pipe[1], postData.c_str(), postData.length());
    close(stdin_pipe[1]);

    // Read the script's output
    std::string output;
    char buffer[4096];
    int bytes_read;
    while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer))) > 0) {
        output.append(buffer, bytes_read);
    }
    close(pipe_fd[0]);

    int status;
    waitpid(pid, &status, 0);

    return (WIFEXITED(status) && WEXITSTATUS(status) == 0) ? output : "500";
}


void CGI::printCGI() {
    std::cout << "CGI pid: " << this->pid << std::endl;
    std::cout << "CGI env:" << std::endl;
    for (std::map<std::string, std::string>::iterator it = env.begin(); it != env.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }
}
