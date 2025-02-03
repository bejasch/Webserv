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

    // Prepare the POST data
    std::string postData;
    if (formData.count("name") && formData.count("message")) {
        postData = "name=" + formData.at("name") + "&message=" + formData.at("message");
    } else {
        return "500"; // Return an error if required fields are missing
    }

    // Create two pipes: one for input (stdin) and one for output (stdout)
    int inputPipe[2], outputPipe[2];
    if (pipe(inputPipe) == -1 || pipe(outputPipe) == -1) {
        perror("pipe");
        return "500";
    }

    // Fork to execute the CGI script
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return "500";
    } else if (pid == 0) {
        // Child process

        // Redirect stdin to read from the input pipe
        dup2(inputPipe[0], STDIN_FILENO);
        close(inputPipe[0]);
        close(inputPipe[1]); // Close unused write end

        // Redirect stdout to write to the output pipe
        dup2(outputPipe[1], STDOUT_FILENO);
        close(outputPipe[1]);
        close(outputPipe[0]); // Close unused read end

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
        // Parent process

        // Close unused ends of pipes
        close(inputPipe[0]);  // Parent doesn't read from input pipe
        close(outputPipe[1]); // Parent doesn't write to output pipe

        // Write the POST data to the CGI script's stdin
        write(inputPipe[1], postData.c_str(), postData.length());
        close(inputPipe[1]); // Close write end after writing

        // Wait for the CGI script to finish
        int status;
        waitpid(pid, &status, 0);

        // Read the output of the CGI script
        char buffer[1024];
        ssize_t bytesRead = read(outputPipe[0], buffer, sizeof(buffer));
        close(outputPipe[0]); // Close read end after reading

        if (bytesRead > 0) {
            std::cout << "Read " << bytesRead << " bytes from CGI script" << std::endl;
            return std::string(buffer, bytesRead);
        } else {
            std::cerr << "Failed to read from CGI script" << std::endl;
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
