#include "../include/CGI.hpp"

static char **makeenv(const Router::Decision &decision, const Request &req) {
	char **env = new char*[8];
	//fill the env with all the necessary CGI variables
	if (decision.fsPath.find("?") != std::string::npos) {
		env[0] = strdup(("QUERY_STRING=" + decision.fsPath.substr(decision.fsPath.find("?") + 1)).c_str());
	}
	else {
		env[0] = strdup("QUERY_STRING=");
	}
	env[1] = strdup(("REQUEST_METHOD=" + req.getMethod()).c_str());
	env[2] = strdup(("CONTENT_LENGTH=" + req.getHeader("Content-Length")).c_str());
	env[3] = strdup(("SCRIPT_NAME=" + decision.fsPath).c_str());
	env[4] = strdup(("PATH_INFO=" + decision.fsPath.substr(0, decision.fsPath.find_last_of("/"))).c_str());
	env[5] = strdup(("CONTENT_TYPE=" + req.getHeader("Content-Type")).c_str());
	env[6] = strdup("PATH=/usr/bin:/bin");
	env[7] = NULL;
	return env;
}

static void freeenv(char **env) {
	for (int i = 0; env[i]; i++) {
		 free(env[i]);
	}
	delete[] env;
}

static bool parseCgiOutput(Response &res, const std::string &raw) {
	size_t header_end = raw.find("\r\n\r\n");
	if (header_end == std::string::npos) {
		return false;
	}
	std::string header = raw.substr(0, header_end);
	std::string body = raw.substr(header_end + 4);
	size_t pos = 0;
	while (pos < header.size()) {
		size_t line_end = header.find("\r\n", pos);
		if (line_end == std::string::npos) {
			line_end = header.size();
		}
		std::string line = header.substr(pos, line_end - pos);
		if (line.find("Status:") == 0) {
			size_t space_pos = line.find(" ");
			if (space_pos != std::string::npos) {
				int status = atoi(line.substr(space_pos + 1).c_str());
				res.setStatus(status, ""); // Reason phrase is optional
			}
		} else {
			size_t colon_pos = line.find(":");
			if (colon_pos != std::string::npos) {
				std::string key = line.substr(0, colon_pos);
				std::string value = line.substr(colon_pos + 1);
				// Trim leading spaces
				value.erase(0, value.find_first_not_of(" "));
				res.setHeader(key, value);
				res.setStatus(200, "OK"); // Default to 200 if no Status header
			}
		}
		pos = line_end + 2;
	}
	res.setBody(body);
	res.setHeader("Content-Length", std::to_string(body.size()));
	return true;
}

Response CGI::run(const Router::Decision &decision, const Request &req) {
	Response res;
	char **env = makeenv(decision, req);
	// validate decision
	if (decision.type != Router::ACTION_CGI) {
		res.setStatus(500, "Internal Server Error");
		res.setBody("<html><body><h1>500 Internal Server Error</h1><p>CGI execution error: Invalid action type.</p></body></html>");
		res.setHeader("Content-Type", "text/html");
		res.setHeader("Content-Length", std::to_string(res.getBody().size()));
		freeenv(env);
		return res;
	}
	int pipe_in[2];
	int pipe_out[2];
	if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1) {
		res.setStatus(500, "Internal Server Error");
		res.setBody("<html><body><h1>500 Internal Server Error</h1><p>CGI execution error: Pipe creation failed.</p></body></html>");
		res.setHeader("Content-Type", "text/html");
		freeenv(env);
		return res;
	}
	// fork process
	pid_t pid = fork();
	if (pid < 0) {
		res.setStatus(500, "Internal Server Error");
		res.setBody("<html><body><h1>500 Internal Server Error</h1><p>CGI execution error: Fork failed.</p></body></html>");
		res.setHeader("Content-Type", "text/html");
		close(pipe_in[0]);
		close(pipe_in[1]);
		close(pipe_out[0]);
		close(pipe_out[1]);
		freeenv(env);
		return res;
	}
	// child process
	if (pid == 0)
	{
		dup2(pipe_in[0], STDIN_FILENO);
		dup2(pipe_out[1], STDOUT_FILENO);
		close(pipe_in[1]);
		close(pipe_out[0]);
		std::vector<char *> argv;
		argv.push_back(strdup(decision.fsPath.c_str()));
		argv.push_back(NULL);
		execve(decision.fsPath.c_str(), &argv[0], env);
		// CHILD, après execve qui échoue
		const char* msg =
			"Status: 500 Internal Server Error\r\n"
			"Content-Type: text/plain\r\n"
			"\r\n\r\n"
			"execve failed\r\n";
		write(STDOUT_FILENO, msg, strlen(msg));
		_exit(1);
	}
	close(pipe_in[0]);
	close(pipe_out[1]);

	// if post request write body to stdin of cgi
	if (req.getMethod() == "POST") {
		size_t toWrite = req.getBody().size();
		const char *body = req.getBody().c_str();
		while (toWrite > 0) {
			ssize_t written = write(pipe_in[1], body, toWrite);
			if (written < 0) {
				if (errno == EINTR)
					continue;
				break;
			}
			toWrite -= written;
			body += written;
		}
	}
	//close writing end of stdin
	close(pipe_in[1]);

	std::string raw;
	char buffer[4096];
	for (;;) {
		ssize_t count = read(pipe_out[0], buffer, sizeof(buffer));
		if (count < 0) {
			if (errno == EINTR)
				continue;
			break;
		}
		if (count == 0)
			break;
		raw.append(buffer, count);
	}
	close(pipe_out[0]);
	int status;
	waitpid(pid, &status, 0);
	freeenv(env);
	if (raw.empty()) {
		res.setStatus(500, "Internal Server Error");
		res.setBody("<html><body><h1>500 Internal Server Error</h1><p>CGI execution error: No output from CGI script.</p></body></html>");
		res.setHeader("Content-Type", "text/html");
		res.setHeader("Content-Length", std::to_string(res.getBody().size()));
		return res;
	}
	if (!parseCgiOutput(res, raw)) {
		res.setStatus(500, "Internal Server Error");
		res.setBody("<html><body><h1>500 Internal Server Error</h1><p>CGI execution error: Malformed CGI output.</p></body></html>");
		res.setHeader("Content-Type", "text/html");
		res.setHeader("Content-Length", std::to_string(res.getBody().size()));
		return res;
	}
	// res.setStatus(200, "OK");
	if (res.getHeaders()["Content-Type"].empty()) {
		res.setHeader("Content-Type", "text/html");
	}
	if (res.getHeaders()["Content-Length"].empty()) {
		res.setHeader("Content-Length", std::to_string(res.getBody().size()));
	}
	return res;
}
