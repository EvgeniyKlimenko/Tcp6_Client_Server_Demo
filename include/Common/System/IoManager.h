#if !defined(__IO_MANAGER_H__)
#define __IO_MANAGER_H__

#include "CommonDefinitions.h"
#include "System/Endpoint.h"

#if defined(_WIN64)

// An IO completion port wrapping class.
class CIoManager final
{
public:
	// Create new completion port.
	CIoManager(size_t threadCount);
	~CIoManager();

	// Bind endpoint to a completion port.
	void Bind(IEndpoint* pEndpoint);
	// Post exit signal to finish up thread routines.
	void Stop();

	// Data coming to particular endpoint post-processed here.
	void Run();

private:
	HANDLE m_hPort;
	size_t m_threadCount;
};


#elif defined(__linux__)

class IoManager final
{
	// Use this wrapper class to facilitate adding/removing epoll events. 
	class EventWrapper
	{
		int m_fd;
	public:
		EventWrapper() : m_fd(0) {}
		void Set(int fd) { m_fd = fd; }
		void DoOp(int opcode, uint32_t events, IEndpoint* endpoint);
		void DoOp(int opcode, uint32_t events, int fd, void* context = nullptr);
	};
public:
	// Create new epoll.
	IoManager(size_t threadCount);
	~IoManager();

	// Bind endpoint to an epoll.
	void Bind(IEndpoint* endpoint);
	// Unbind endpoint from an epoll.
	void Unbind(IEndpoint* endpoint);
	// Post exit signal to finish up thread routines.
	void Stop();

	// Data coming to particular endpoint post-processed here.
	void Run();

private:
	enum ExitEnds {reader, writer};
	static const size_t MAX_ENDPOINTS = 0xffff;
	size_t m_threadCount;
	int m_fd;
	int m_exiters[2];
	EventWrapper m_ewr;
};

#endif // _WIN64

#endif // __IO_MANAGER_H__