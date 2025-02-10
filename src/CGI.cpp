#include "../headers/AllHeaders.hpp"

CGI::CGI() : pid(0), env({}), envp(nullptr), argv(nullptr) {
    std::cout << "CGI default constructor called" << std::endl;
}

CGI::~CGI() {
    freeEnvironment();
    std::cout << "CGI destructor called" << std::endl;
}

void    CGI::setAllEnv(HttpRes &httpResponse) {
    env["SERVER_PROTOCOL"] = "HTTP/1.1";
    env["REQUEST_METHOD"] = httpResponse.getMethod();
    env["SCRIPT_NAME"] = httpResponse.getTarget();
    env["DOCUMENT_ROOT"] = httpResponse.getRoute()->getRootDirRoute();
    this->envp = new char*[this->env.size() + 1];
    if (this->envp == nullptr) {
        perror("Failed to allocate memory for envp");
        return;
    }
	std::map<std::string, std::string>::const_iterator it = this->env.begin();
	for (int i = 0; it != this->env.end(); it++, i++)
	{
		std::string tmp = it->first + "=" + it->second;
		this->envp[i] = new char[tmp.size() + 1];
        std::copy(tmp.begin(), tmp.end(), this->envp[i]);
        this->envp[i][tmp.size()] = '\0';
	}
    this->envp[this->env.size()] = nullptr;
    this->argv = new char*[3]{nullptr, nullptr, nullptr};
    if (this->envp == nullptr || this->argv == nullptr) {
        perror("Failed to allocate memory for envp or argv");
        return;
    }
}

//TODO: what happens when no .py or .php configs are set
std::string CGI::executeCGI_GET(HttpRes &httpResponse) {
    setAllEnv(httpResponse);
    std::string scriptPath = env["DOCUMENT_ROOT"] + env["SCRIPT_NAME"];
    std::cout << "Executing CGI script (GET): " << scriptPath << std::endl;

    if (getFileExtension(scriptPath) == ".py") {
        std::string interpreter = "/usr/bin/python3";
        this->argv[0] = new char[interpreter.size() + 1];
        if (this->argv[0] == nullptr) {
            perror("Failed to allocate memory for argv[0]");
            return "500";  // Internal Server Error
        }
        std::copy(interpreter.begin(), interpreter.end(), this->argv[0]);
        this->argv[0][interpreter.size()] = '\0';
    } 
    else if (getFileExtension(scriptPath) == ".php") {
        std::string interpreter = "/usr/bin/php";
        this->argv[0] = new char[interpreter.size() + 1];
        if (this->argv[0] == nullptr) {
            perror("Failed to allocate memory for argv[0]");
            return "500";  // Internal Server Error
        }
        std::copy(interpreter.begin(), interpreter.end(), this->argv[0]);
        this->argv[0][interpreter.size()] = '\0';
    }
    // Properly allocate argv[1] as well
    this->argv[1] = new char[scriptPath.size() + 1];
    if (this->argv[1] == nullptr) {
        perror("Failed to allocate memory for argv[1]");
        return "500";  // Internal Server Error
    }
    std::copy(scriptPath.begin(), scriptPath.end(), this->argv[1]); 
    this->argv[1][scriptPath.size()] = '\0';  // Null-terminate

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

        if (execve(argv[0], argv, envp) == -1) {
            perror("Failed to execute script");
            exit(EXIT_FAILURE);
        }
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

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return output;
    } else {
        std::cout << "CGI script exited with status " << WEXITSTATUS(status) << std::endl;
        return "500";
    }
}

std::string CGI::executeCGI_POST(HttpRes &httpResponse, const std::map<std::string, std::string> &formData) {
    std::string scriptPath;
    
     setAllEnv(httpResponse);
    // Check if the "action" key exists in formData
    auto it = formData.find("action");
    if (it != formData.end()) {
        if (it->second == "Scramble.py") {
            scriptPath = "data/cgi-bin/modify_comments.py";
            this->argv[0] = strdup("/usr/bin/python3");
        }
        else if (it->second == "Capitalize.php"){
            scriptPath = "data/cgi-bin/modify_comments.php";
            this->argv[0] = strdup("/usr/bin/php");
        }
        this->argv[1] = strdup(scriptPath.c_str());
    }
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

        // Execute the CGI script using execve
        if (execve(argv[0], argv, envp) == -1) {
            perror("execve failed");
            exit(1);
        }
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
    return "500";
}

void CGI::printCGI() {
    std::cout << "CGI pid: " << this->pid << std::endl;
    std::cout << "CGI env:" << std::endl;
    for (std::map<std::string, std::string>::iterator it = env.begin(); it != env.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }
}

void CGI::freeEnvironment() {
    if (envp) {
        for (int i = 0; envp[i]; i++) {
            delete[] envp[i];
        }
        delete[] envp;
        envp = nullptr;
    }

    if (argv) {
        for (int i = 0; argv[i]; i++) {
            delete[] argv[i];
        }
        delete[] argv;
        argv = nullptr;
    }
    env.clear();
}
