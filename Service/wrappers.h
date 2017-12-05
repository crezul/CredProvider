
#pragma once 
#include "inclfiles.h"

class HandleGuard {
	HANDLE handle_;
public:
	inline HandleGuard(HANDLE handle)
		: handle_(handle) {}
	inline ~HandleGuard()
	{
		CloseHandle(handle_);
	}
};


class SocketGuard {
	unsigned __int64 *socket_;
public:
	inline SocketGuard(unsigned __int64* socket)
		: socket_(socket) {}
	     ~SocketGuard()
	{
		shutdown(*socket_, 2);

		if (socket_)
			delete socket_;
		socket_ = NULL;
	}
};
