#ifndef UPLOAD_HPP

#define UPLOAD_HPP

#include "Router.hpp"
#include "Response.hpp"

class Upload
{
	public :
		// Produce an upload response (blocking I/O):
		static Response save(const Router::Decision& d, const Request& req);
};

#endif
