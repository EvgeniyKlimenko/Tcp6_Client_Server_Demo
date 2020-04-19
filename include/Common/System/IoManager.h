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
	// Exiting eventfd wrapped into endpoint.
	class Exiter : public IEndpoint
	{
		int m_fd;
	public:
		Exiter();
		virtual ~Exiter() { close(m_fd); }

		int Get() override { return m_fd; }
		bool Complete() override { Signal(); return false; }

		void Signal(bool first = false);
	};

	// Use this wrapper class to facilitate adding/removing epoll events. 
	class EventWrapper
	{
		int m_fd;
	public:
		EventWrapper() : m_fd(0) {}
		void Set(int fd) { m_fd = fd; }
		void DoOp(int opcode, uint32_t events, IEndpoint* endpoint);
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
	static const size_t MAX_ENDPOINTS = 0xffff;
	boost::atomic<size_t> m_threadCount;
	int m_fd;
	Exiter m_exiter;
	EventWrapper m_ewr;
};

#endif // _WIN64

#endif // __IO_MANAGER_H__