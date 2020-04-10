#include "System/IoManager.h"
#include "System/Exception.h"

#if defined(_WIN64)

CIoManager::CIoManager(size_t threadCount)
: m_threadCount(threadCount)
{
	// Create empty completion port object.
	m_hPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	if (!m_hPort)
		throw CWindowsException(GetLastError());
}
	
CIoManager::~CIoManager()
{
	Stop();
	CloseHandle(m_hPort);
}

void CIoManager::Bind(IEndpoint* pEndpoint)
{
	if(!pEndpoint || !pEndpoint->IsValid())
		throw CWindowsException(ERROR_INVALID_PARAMETER);

	// Associate endpoint with a completion port.
	if(!CreateIoCompletionPort(reinterpret_cast<HANDLE>(pEndpoint->Get()), m_hPort,
		reinterpret_cast<ULONG_PTR>(pEndpoint), 0))
	{
		throw CWindowsException(GetLastError());
	}
}

void CIoManager::Stop()
{
	// Send finish to all threads bound to the port.
	// The last parameter is context of null value.
	for(ULONG i = 0; i < m_threadCount; ++i)
		PostQueuedCompletionStatus(m_hPort, 0, 0, nullptr);
}
	
void CIoManager::Run()
{
	// Set SEH exception wrapper.
	CSehException::Setup();

	for (;;)
	{
		ULONG bytesTransferred = 0;
		IEndpoint* pEndpoint = nullptr;
		LPOVERLAPPED pContext = nullptr;

		// Start processing of the next data portion.
		if (!GetQueuedCompletionStatus(m_hPort, &bytesTransferred, reinterpret_cast<PULONG_PTR>(&pEndpoint),
			&pContext, INFINITE))
		{
			throw CWindowsException(GetLastError());
		}

		// A null context passed by finishing.
		if (!pContext)
			break;

		// Data arrived for certain endpoint. Each endpoint is aware how to complete this data portion.
		pEndpoint->Complete(bytesTransferred);
	}
}

#elif defined(__linux__)

IoManager::IoManager(size_t threadCount)
: m_threadCount(threadCount)
{
	// Creating epoll itself.
	m_fd = epoll_create1(0);
	if(m_fd < 0) throw SystemException(errno);

	// Creating pipes to signal epoll at exit to be woken up from waiting.
	if(pipe2(m_exiters, O_NONBLOCK) < 0)
		throw SystemException(errno);

	// Bind exit reader with an epoll.
	epoll_event ev;
	ev.events = EPOLLET | EPOLLIN;
	ev.data.fd = m_exiters[reader];
	if(epoll_ctl(m_fd, EPOLL_CTL_ADD, m_exiters[reader], &ev) < 0)
		throw SystemException(errno);
}

IoManager::~IoManager()
{
	Stop();
	close(m_exiters[writer]);
	close(m_exiters[reader]);
	close(m_fd);
}

void IoManager::Bind(IEndpoint* endpoint)
{
	epoll_event ev;
	ev.events = EPOLLET | EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLEXCLUSIVE | EPOLLWAKEUP;
	ev.data.fd = endpoint->Get();
	ev.data.ptr = endpoint;
	if(epoll_ctl(m_fd, EPOLL_CTL_ADD, endpoint->Get(), &ev) < 0)
		throw SystemException(errno);
}

void IoManager::Stop()
{
	// Send finish to all threads bound to epoll via exit writer.
	// Exit reader in turn will be signaled waking up
	// the thread and than breaking a thread loop.
	for(size_t i = 0; i < m_threadCount; ++i)
	{
		bool exit = true;
		write(m_exiters[writer], &exit, sizeof(exit));
	}
}

void IoManager::Run()
{
    boost::array<epoll_event, MAX_ENDPOINTS> events;

    for (;;)
    {
        int readyCount = epoll_wait(m_fd, events.data(), MAX_ENDPOINTS, -1);
		if (readyCount < 0) throw SystemException(errno);

        for (int i = 0; i < readyCount; ++i)
        {
			if (events[i].data.fd == m_exiters[reader])
			{
				// Time to finish.
				return;
			}

			// Asynchronous operation occurred on endpoint needed to complete.
			IEndpoint* e = reinterpret_cast<IEndpoint*>(events[i].data.ptr);
			e->Complete();
        }
    }
}

#endif // _WIN64