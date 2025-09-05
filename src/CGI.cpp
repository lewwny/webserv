#include "../include/CGI.hpp"
#include <sstream>

// C++98 compatible string conversion function
static std::string size_to_string(size_t value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

static void splitFsPathQuery(const std::string &fsPath, std::string &scriptPath, std::string &query) {
	size_t pos = fsPath.find("?");
	if (pos == std::string::npos) {
		scriptPath = fsPath;
		query = "";
	} else {
		scriptPath = fsPath.substr(0, pos);
		query = fsPath.substr(pos + 1);
	}
}

static std::string computePathInfo(const std::string &relPath, const std::string &cgiExt)
{
	if (cgiExt.empty()) {
		return std::string();
	}
	size_t pos = relPath.rfind(cgiExt);
	if (pos == std::string::npos) {
		return std::string();
	}
	size_t pathInfoStart = pos + cgiExt.size();
	if (pathInfoStart >= relPath.size()) {
		return std::string();
	}
	std::string pathInfo = relPath.substr(pathInfoStart);
	if (!pathInfo.empty() && pathInfo[0] != '/') {
		pathInfo.insert(pathInfo.begin(), '/');
	}
	return pathInfo;
}

static std::string joinUrl(const std::string& a, const std::string& b) {
	if (a.empty()) return b;
	if (b.empty()) return a;
	if (a[a.size() - 1] == '/' && b[0] == '/') {
		return a + b.substr(1);
	} else if (a[a.size() - 1] != '/' && b[0] != '/') {
		return a + "/" + b;
	} else {
		return a + b;
	}
}

static char **makeenv(const Router::Decision &d, const Request &req,
					  const std::string& scriptPath, const std::string& query,
					  std::string& outScriptName, std::string& outRequestUri)
{
	std::string rel = d.relPath;
	std::string scriptName = d.mountUri;
	if (!rel.empty()) {
		size_t cut = (d.cgiExt.empty()) ? rel.size() : rel.find(d.cgiExt);
		if (cut == std::string::npos) cut = rel.size();
		std::string relToScript = rel.substr(0, cut + (d.cgiExt.empty() ? 0 : d.cgiExt.size()));
		scriptName = joinUrl(scriptName, relToScript);
	}
	if (scriptName.empty())
		scriptName = "/";
	std::string pathInfo = computePathInfo(rel, d.cgiExt);
	std::string requestUri = scriptName + pathInfo + (query.empty() ? "" : ("?" + query));
	std::string pathTranslated;
	if (!pathInfo.empty() && !d.root.empty()) {
		pathTranslated = d.root;
		if (!pathTranslated.empty() && pathTranslated[pathTranslated.size() - 1] != '/') pathTranslated += '/';
		pathTranslated += pathInfo[0] == '/' ? pathInfo.substr(1) : pathInfo;
	}
	std::string serverProtocol = "HTTP/1.1";
	std::string serverName     = "localhost";
	std::string serverPort     = "80";
	std::string remoteAddr     = "127.0.0.1";
	std::string contentLength = req.getHeader("Content-Length");
	std::string contentType   = req.getHeader("Content-Type");

	std::vector<std::string> kv;
	kv.push_back("GATEWAY_INTERFACE=CGI/1.1");
	kv.push_back("SERVER_PROTOCOL=" + serverProtocol);
	kv.push_back("SERVER_NAME=" + serverName);
	kv.push_back("SERVER_PORT=" + serverPort);
	kv.push_back("REQUEST_METHOD=" + req.getMethod());
	kv.push_back("SCRIPT_NAME=" + scriptName);
	kv.push_back("SCRIPT_FILENAME=" + scriptPath);
	kv.push_back("REQUEST_URI=" + requestUri);
	kv.push_back("QUERY_STRING=" + query);
	kv.push_back("PATH_INFO=" + pathInfo);
	if (!pathTranslated.empty()) kv.push_back("PATH_TRANSLATED=" + pathTranslated);
	kv.push_back("REMOTE_ADDR=" + remoteAddr);
	kv.push_back("CONTENT_LENGTH=" + contentLength);
	kv.push_back("CONTENT_TYPE=" + contentType);
	kv.push_back("PATH=/usr/bin:/bin");
	std::string conn = req.getHeader("Connection");
	if (!conn.empty())
		kv.push_back("HTTP_CONNECTION=" + conn);
	std::string host = req.getHeader("Host");
	if (!host.empty())
		kv.push_back("HTTP_HOST=" + host);
	std::string ua = req.getHeader("User-Agent");
	if (!ua.empty())
		kv.push_back("HTTP_USER_AGENT=" + ua);
	char **env = new char*[kv.size() + 1];
	for (size_t i = 0; i < kv.size(); i++)
		env[i] = strdup(kv[i].c_str());
	env[kv.size()] = NULL;
	outScriptName = scriptName;
	outRequestUri = requestUri;
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
	size_t sep_len = 4;
	if (header_end == std::string::npos) {
		header_end = raw.find("\n\n");
		sep_len = 2;
	}
	if (header_end == std::string::npos)
		return false;

	std::string header = raw.substr(0, header_end);
	std::string body   = raw.substr(header_end + sep_len);

	bool hasStatus = false;
	res.setStatus(200, "OK");

	size_t pos = 0;
	while (pos < header.size()) {
		size_t line_end = header.find('\n', pos);
		if (line_end == std::string::npos)
			line_end = header.size();
		std::string line = header.substr(pos, line_end - pos);
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		if (!line.empty()) {
			if (line.rfind("Status:", 0) == 0) {
				size_t sp = line.find(' ');
				if (sp != std::string::npos) {
					int status = atoi(line.substr(sp + 1).c_str());
					res.setStatus(status, "");
					hasStatus = true;
				}
			} else {
				size_t cp = line.find(':');
				if (cp != std::string::npos) {
					std::string key = line.substr(0, cp);
					std::string value = line.substr(cp + 1);
					size_t i = value.find_first_not_of(" \t");
					if (i != std::string::npos)
						value.erase(0, i);
					res.setHeader(key, value);
				}
			}
		}
		pos = (line_end == header.size()) ? line_end : line_end + 1;
	}

	const std::map<std::string, std::string>& H = res.getHeaders();
	if (!hasStatus && (H.find("Location") != H.end())) {
		res.setStatus(302, "Found");
	}

	res.setBody(body);
	res.setHeader("Content-Length", size_to_string(body.size()));
	return true;
}


Response CGI::run(const Router::Decision &decision, const Request &req) {
	Response res;
	// validate decision
	if (decision.type != Router::ACTION_CGI) {
		res.setStatus(500, "Internal Server Error");
		res.setBody("<html><body><h1>500 Internal Server Error</h1><p>CGI execution error: Invalid action type.</p></body></html>");
		res.setHeader("Content-Type", "text/html");
		res.setHeader("Content-Length", size_to_string(res.getBody().size()));
		return res;
	}
	std::string scriptPath, query;
	splitFsPathQuery(decision.fsPath, scriptPath, query);
	std::string scriptName, requestUri;
	char **env = makeenv(decision, req, scriptPath, query, scriptName, requestUri);
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
		dup2(pipe_out[1], STDERR_FILENO);
		close(pipe_in[1]);
		close(pipe_out[0]);
		std::vector<char *> argv;
		if (!decision.cgiInterpreter.empty()) {
			argv.push_back(strdup(decision.cgiInterpreter.c_str()));
			argv.push_back(strdup(scriptPath.c_str()));
		}
		else {
			argv.push_back(strdup(scriptPath.c_str()));
		}
		argv.push_back(NULL);
		if (!decision.cgiInterpreter.empty()) {
			//if cgi interpreter is set, we execute interpreter with script as argument
			execve(decision.cgiInterpreter.c_str(), &argv[0], env);
		} else {
			// else we execute script directly
			execve(scriptPath.c_str(), &argv[0], env);
		}
		// CHILD, après execve qui échoue
		const char* msg =
			"Status: 500 Internal Server Error\r\n"
			"Content-Type: text/plain\r\n"
			"\r\n\r\n"
			"execve failed\r\n";
		write(STDOUT_FILENO, msg, strlen(msg));
		close(pipe_in[0]);
		close(pipe_out[1]);
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
	raw.reserve(8192);
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
		res.setHeader("Content-Length", size_to_string(res.getBody().size()));
		return res;
	}
	if (!parseCgiOutput(res, raw)) {
		res.setStatus(500, "Internal Server Error");
		res.setBody("<html><body><h1>500 Internal Server Error</h1><p>CGI execution error: Malformed CGI output.</p></body></html>");
		res.setHeader("Content-Type", "text/html");
		res.setHeader("Content-Length", size_to_string(res.getBody().size()));
		return res;
	}
	// res.setStatus(200, "OK");
	if (res.getHeaders()["Content-Type"].empty()) {
		res.setHeader("Content-Type", "text/html");
	}
	if (res.getHeaders()["Content-Length"].empty()) {
		res.setHeader("Content-Length", size_to_string(res.getBody().size()));
	}
	return res;
}
