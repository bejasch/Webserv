#include "../headers/AllHeaders.hpp"

CGI::CGI() : pid(0), env(), envp(NULL), argv(NULL) {
	std::cout << "CGI default constructor called" << std::endl;
}

CGI::CGI(const CGI &other) : pid(other.pid), env(other.env), envp(NULL), argv(NULL) {
	std::cout << "CGI copy constructor called" << std::endl;
}

CGI CGI::operator=(const CGI &another) {
	if (this == &another)
		return (*this);
	pid = another.pid;
	env = another.env;
	envp = NULL;
	argv = NULL;
	return (*this);
}

CGI::~CGI() {
	freeEnvironment();
	std::cout << "CGI destructor called" << std::endl;
}

int CGI::setAllEnv(HttpRes &httpResponse) {
	env["SERVER_PROTOCOL"] = "HTTP/1.1";
	env["REQUEST_METHOD"] = httpResponse.getMethod();
	env["SCRIPT_NAME"] = httpResponse.getTarget();
	env["DOCUMENT_ROOT"] = httpResponse.getRoute()->getRootDirRoute();
	this->envp = new char*[this->env.size() + 1];
	if (this->envp == NULL) {
		std::cerr << "Failed to allocate memory for envp: " << std::strerror(errno) << std::endl;
		return(1);
	}
	std::map<std::string, std::string>::const_iterator it = this->env.begin();
	for (int i = 0; it != this->env.end(); it++, i++)
		this->envp[i] = cpp_strdup(it->first + "=" + it->second);
	this->envp[this->env.size()] = NULL;
	this->argv = new char*[3];
	if (this->argv == NULL) {
		std::cerr << "Failed to allocate memory for argv: " << std::strerror(errno) << std::endl;
		return(1);
	}
	this->argv[0] = NULL;
	this->argv[1] = NULL;
	this->argv[2] = NULL;
	return (0);
}

std::string CGI::executeCGI_GET(HttpRes &httpResponse) {
	if (setAllEnv(httpResponse)){
		std::cerr << "Failed to set env variables: " << std::strerror(errno) << std::endl;
		httpResponse.setStatus(500);
		return "";
	}
	std::cout << "root dir: " << env["DOCUMENT_ROOT"] << std::endl;
	std::string scriptPath = env["DOCUMENT_ROOT"] + env["SCRIPT_NAME"];
	std::cout << "Executing CGI script (GET): " << scriptPath << std::endl;

	if (getFileExtension(scriptPath) == ".py")
		this->argv[0] = cpp_strdup("/usr/bin/python3");
	else if (getFileExtension(scriptPath) == ".php")
		this->argv[0] = cpp_strdup("/usr/bin/php");
	this->argv[1] = cpp_strdup(scriptPath);

	int pipe_fd[2];
	if (pipe(pipe_fd) == -1) {
		std::cerr << "Failed to create pipe: " << std::strerror(errno) << std::endl;
		httpResponse.setStatus(500);
		return "";
	}

	pid = fork();
	if (pid == -1) {
		std::cerr << "Failed to fork: " << std::strerror(errno) << std::endl;
		httpResponse.setStatus(500);
		return "";
	}

	if (pid == 0) {  // Child process
		alarm(10); //TODO: is there a cpp equivalent?
		close(pipe_fd[0]);
		dup2(pipe_fd[1], STDOUT_FILENO);
		dup2(pipe_fd[1], STDERR_FILENO);
		close(pipe_fd[1]);

		if (execve(argv[0], argv, envp) == -1) {
			std::cerr << "Failed to execute script: " << std::strerror(errno) << std::endl;
			freeEnvironment();
			ServerManager &serverManager = httpResponse.getServer()->getServerManager();
			serverManager.freeResources();
			exit(EXIT_FAILURE);
		}
	} 

	close(pipe_fd[1]);

	// Make the pipe non-blocking
    fcntl(pipe_fd[0], F_SETFL, O_NONBLOCK);

    // Add pipe to epoll
    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = pipe_fd[0];
    
    if (epoll_ctl(httpResponse.getServer()->getServerManager().getEpollFd(), 
                  EPOLL_CTL_ADD, pipe_fd[0], &ev) == -1) {
        std::cerr << "Failed to add pipe to epoll" << std::endl;
        close(pipe_fd[0]);
        httpResponse.setStatus(500);
        return "";
    }

	char buffer[4096];
	std::string output;
    time_t start_time = time(NULL);
    
    // Read from pipe using epoll
    while (true) {
        epoll_event events[1];
        int nfds = epoll_wait(httpResponse.getServer()->getServerManager().getEpollFd(), 
                             events, 1, 1000); // 1 second timeout

        // Check for timeout
        if (time(NULL) - start_time > 10) {
            kill(pid, SIGTERM);
            usleep(1000);
            kill(pid, SIGKILL);
            epoll_ctl(httpResponse.getServer()->getServerManager().getEpollFd(), 
                     EPOLL_CTL_DEL, pipe_fd[0], NULL);
            close(pipe_fd[0]);
            httpResponse.setStatus(504);
            return "CGI script timed out";
        }

        if (nfds > 0) {
            int bytes_read = read(pipe_fd[0], buffer, sizeof(buffer));
            if (bytes_read > 0) {
                output.append(buffer, bytes_read);
            } else if (bytes_read == 0 || (bytes_read == -1 && errno != EAGAIN)) {
                break;
            }
        }
    }
	// Cleanup
    epoll_ctl(httpResponse.getServer()->getServerManager().getEpollFd(), 
              EPOLL_CTL_DEL, pipe_fd[0], NULL);
	close(pipe_fd[0]);
	int status;
	if (waitpid(pid, &status, WNOHANG) == 0) {
        kill(pid, SIGTERM);
        usleep(1000);
        kill(pid, SIGKILL);
        waitpid(pid, &status, 0);
    }
	if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
		return output;
	} else {
		std::cout << "CGI script exited with status " << WEXITSTATUS(status) << std::endl;
		httpResponse.setStatus(500);
		return "";
	}
}

std::string CGI::executeCGI_POST(HttpRes &httpResponse, const std::map<std::string, std::string> &formData) {
	std::string scriptPath;
	if (setAllEnv(httpResponse)){
		std::cerr << "Failed to set env variables: " << std::strerror(errno) << std::endl;
		httpResponse.setStatus(500);
		return "";
	}
	// Check if the "action" key exists in formData
	std::map<std::string, std::string>::const_iterator it = formData.find("action");
	if (it != formData.end()) {
		if (it->second == "Scramble.py") {
			scriptPath = "data/cgi-bin/modify_comments.py";
			this->argv[0] = cpp_strdup("/usr/bin/python3");
		}
		else if (it->second == "Capitalize.php"){
			scriptPath = "data/cgi-bin/modify_comments.php";
			this->argv[0] = cpp_strdup("/usr/bin/php");
		}
		this->argv[1] = cpp_strdup(scriptPath);
	}
	std::cout << "Executing CGI script (POST): " << scriptPath << std::endl;

	// Prepare the POST data
	std::string postData;
	if (formData.count("name") && formData.count("message")) {
		postData = "name=" + formData.at("name") + "&message=" + formData.at("message");
	} else {
		httpResponse.setStatus(500);
		return "";
	}

	// Create two pipes: one for input (stdin) and one for output (stdout)
	int inputPipe[2], outputPipe[2];
	if (pipe(inputPipe) == -1 || pipe(outputPipe) == -1) {
		std::cerr << "Pipe error: " << std::strerror(errno) << std::endl;
		httpResponse.setStatus(500);
		return "";
	}

	// Fork to execute the CGI script
	pid_t pid = fork();
	if (pid == -1) {
		std::cerr << "Fork error: " << std::strerror(errno) << std::endl;
		httpResponse.setStatus(500);
		return "";
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
			std::cerr << "Failed to execute script: " << std::strerror(errno) << std::endl;
			freeEnvironment();
			ServerManager &serverManager = httpResponse.getServer()->getServerManager();
			serverManager.freeResources();
			exit(EXIT_FAILURE);
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

		if (bytesRead >= 0) {
			std::cout << "Read " << bytesRead << " bytes from CGI script" << std::endl;
			return std::string(buffer, bytesRead);
		} else {
			std::cerr << "Failed to read from CGI script" << std::endl;
			httpResponse.setStatus(500);
			return "";
		}
	}
	httpResponse.setStatus(500);
	return "";
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
		envp = NULL;
	}

	if (argv) {
		for (int i = 0; argv[i]; i++) {
			delete[] argv[i];
		}
		delete[] argv;
		argv = NULL;
	}
	env.clear();
}
