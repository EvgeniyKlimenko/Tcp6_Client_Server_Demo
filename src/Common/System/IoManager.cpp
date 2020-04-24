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
	if(!pEndpoint) throw CWindowsException(ERROR_INVALID_PARAMETER);

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

IoManager::Exiter::Exiter()
{
	// Creating eventfd to signal epoll at exit to be woken up from waiting.
	m_fd = eventfd(0, EFD_NONBLOCK);
	if(m_fd < 0)
		throw SystemException(errno);
}

void IoManager::Exiter::Signal(bool first)
{
	eventfd_t val = 1;
	if (!first)
	{
		// Reading data from exiter.
		if (eventfd_read(m_fd, &val) < 0)
			throw SystemException(errno);
	}

	// Writing data to exiter to wake up next thread.
	if (eventfd_write(m_fd, val) < 0)
		throw SystemException(errno);
}

void IoManager::EventWrapper::DoOp(int opcode, uint32_t events, IEndpoint* endpoint)
{
	epoll_event ev;
	ev.events = events;
	ev.data.fd = endpoint->Get();
	ev.data.ptr = endpoint;
	if (epoll_ctl(m_fd, opcode, endpoint->Get(), &ev) < 0)
		throw SystemException(errno);
}

IoManager::IoManager(size_t threadCount)
: m_threadCount(threadCount)
{
	// Creating epoll itself.
	m_fd = epoll_create1(0);
	if(m_fd < 0) throw SystemException(errno);

	m_ewr.Set(m_fd);

	// Bind exit reader with an epoll.
	m_ewr.DoOp(EPOLL_CTL_ADD, EPOLLET | EPOLLIN | EPOLLEXCLUSIVE, &m_exiter);
}

IoManager::~IoManager()
{
	Stop();
	m_ewr.DoOp(EPOLL_CTL_DEL, EPOLLET | EPOLLIN | EPOLLEXCLUSIVE, &m_exiter);
	close(m_fd);
}

void IoManager::Bind(IEndpoint* endpoint)
{
	m_ewr.DoOp(EPOLL_CTL_ADD, EPOLLET | EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLEXCLUSIVE | EPOLLWAKEUP, endpoint);
}

void IoManager::Unbind(IEndpoint* endpoint)
{
	// In kernel versions before 2.6.9, the EPOLL_CTL_DEL operation required
    // a non-null pointer in event, even though this argument is ignored.
    // Since Linux 2.6.9, event can be specified as NULL when using
    // EPOLL_CTL_DEL.  Applications that need to be portable to kernels
    // before 2.6.9 should specify a non-null pointer in event.
	m_ewr.DoOp(EPOLL_CTL_DEL, EPOLLET | EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLEXCLUSIVE | EPOLLWAKEUP, endpoint);
}

void IoManager::Stop()
{
	size_t expected = 0;
	if (m_threadCount.compare_exchange_strong(expected, 0, boost::memory_order_relaxed))
		return;

	// If there are no IO-involved threads just return control.
	std::cout << "Thread count: " << m_threadCount.load(boost::memory_order_relaxed) << std::endl;

	// Send finish to all threads bound to epoll via exit writer.
	// Exit reader in turn will be signaled waking up
	// the thread and than breaking a thread loop.
	m_exiter.Signal(true);
}

void IoManager::Run()
{
    for (;;)
    {
		boost::array<epoll_event, MAX_ENDPOINTS> events;
        int readyCount = epoll_wait(m_fd, events.data(), MAX_ENDPOINTS, -1);
		if (readyCount < 0)
		{
			if (errno == EINTR) continue;
			else throw SystemException(errno);
		}

        for (int i = 0; i < readyCount; ++i)
        {
			// Asynchronous operation occurred on endpoint needed to complete.
			IEndpoint* e = reinterpret_cast<IEndpoint*>(events[i].data.ptr);
			if(!e->Complete())
			{
				m_threadCount.fetch_sub(1, boost::memory_order_relaxed);
				return;
			}
        }
    }
}

#endif // _WIN64