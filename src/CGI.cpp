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
       // Prepare the POST data

    std::string scriptPath = "data/cgi-bin/modify_comments.py";
    std::cout << "Executing CGI script (POST): " << scriptPath << std::endl;
    // Prepare the POST data
    std::string postData;
    if (formData.count("name") && formData.count("message")) {
        postData = "name=" + formData.at("name") + "&message=" + formData.at("message");
    } else {
        return "500"; // Return an error if required fields are missing
    }

    // Set the CONTENT_LENGTH environment variable
    setenv("CONTENT_LENGTH", std::to_string(postData.length()).c_str(), 1);

    // Create pipes for communication with the CGI script
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return "500";
    }

    // Fork to execute the CGI script
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return "500";
    } else if (pid == 0) {
        // Child process: execute the CGI script
        close(pipefd[1]); // Close the write end of the pipe
        dup2(pipefd[0], STDIN_FILENO); // Redirect stdin to read from the pipe
        close(pipefd[0]);

        // Prepare arguments for execve
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(scriptPath.c_str())); // Script path
        argv.push_back(nullptr); // Null-terminate the argument list

        // Prepare environment variables for execve
        std::vector<char*> envp;
        for (auto& env_var : env) {
            std::string env_entry = env_var.first + "=" + env_var.second;
            envp.push_back(const_cast<char*>(env_entry.c_str()));
        }
        envp.push_back(nullptr); // Null-terminate the environment list

        // Execute the CGI script using execve
        execve(scriptPath.c_str(), argv.data(), envp.data());
        perror("execve"); // If execve fails
        exit(1);
    } else {
        // Parent process: write the POST data to the CGI script
        close(pipefd[0]); // Close the read end of the pipe
        write(pipefd[1], postData.c_str(), postData.length());
        close(pipefd[1]);

        // Wait for the CGI script to finish
        int status;
        waitpid(pid, &status, 0);

        // Read the output of the CGI script
        char buffer[1024];
        ssize_t bytesRead = read(pipefd[0], buffer, sizeof(buffer));
        if (bytesRead > 0) {
            return std::string(buffer, bytesRead);
        } else {
            return "500";
        }
    }
}


void CGI::printCGI() {
    std::cout << "CGI pid: " << this->pid << std::endl;
    std::cout << "CGI env:" << std::endl;
    for (std::map<std::string, std::string>::iterator it = env.begin(); it != env.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }
}
