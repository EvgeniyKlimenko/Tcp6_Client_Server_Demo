#if !defined(__WINSOCK_INITER_H__)
#define __WINSOCK_INITER_H__

#include "CommonDefinitions.h"

#if defined(_WIN64)

class CWinSockIniter final
{
public:
	CWinSockIniter()
	{
        WSAStartup(MAKEWORD(2, 2), &m_wsaData);
	}

	~CWinSockIniter()
	{
		WSACleanup();
	}

private:
	WSADATA m_wsaData;
};

#endif // _WIN64

#endif // __WINSOCK_INITER_H__