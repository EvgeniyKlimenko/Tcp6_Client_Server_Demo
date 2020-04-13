#include "System/ThreadPool.h"
#include "System/Exception.h"

#if defined(_WIN64)

CThreadPool::CThreadPool(CThreadPool::ThreadCallback_t threadCallback)
: m_threadCallback(threadCallback)
{
	// Determine number of dedicated threads.
	SYSTEM_INFO sysInfo = { 0 };
	GetSystemInfo(&sysInfo);
	m_threadCount = 2 * sysInfo.dwNumberOfProcessors + 1;
}

CThreadPool::~CThreadPool()
{
    Stop();
}

unsigned int __stdcall CThreadPool::_ThreadCallback(void* param)
{
    CThreadPool* self = reinterpret_cast<CThreadPool*>(param);
    self->m_threadCallback();
    return 0;
}

size_t CThreadPool::GetThreadCount() const
{
	return m_threadCount;
}

void CThreadPool::Start()
{
	// Create dedicated threads using VC++ runtime call.
	// We use low level features deliberately
	// holding back from C++11 or Boost classes
	// because this class is system-specific, not cross plaltform.
	for (unsigned i = 0; i < m_threadCount; ++i)
	{
		HANDLE th = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, 
			&CThreadPool::_ThreadCallback, this, 0, nullptr));
		if (!th) throw SystemException(errno);

		m_threads.push_back(th);
	}
}

void CThreadPool::Stop()
{
	// Without active working threads has no sence. 
	if(m_threads.empty()) return;

	// Wait while the dedicated threads finish.
	PHANDLE threadHandles = &m_threads[0];
	WaitForMultipleObjects(m_threadCount, threadHandles, TRUE, INFINITE);

	// Clreanup each thread object.
	std::for_each(std::begin(m_threads), std::end(m_threads),
		[](HANDLE th) {CloseHandle(th);});

	// Clear since ther's no more active threads.
	m_threads.clear();
}

#elif defined(__linux__)

ThreadPool::ThreadPool(ThreadPool::ThreadCallback_t threadCallback)
: m_threadCallback(threadCallback)
{
	// Determine number of dedicated threads.
	m_threadCount = 2 * get_nprocs() + 1;
}

ThreadPool::~ThreadPool()
{
    Stop();
}

void* ThreadPool::_ThreadCallback(void* param)
{
    ThreadPool* self = reinterpret_cast<ThreadPool*>(param);
    self->m_threadCallback();
    return nullptr;
}

size_t ThreadPool::GetThreadCount() const
{
	return m_threadCount;
}

void ThreadPool::Start()
{
	for (unsigned i = 0; i < m_threadCount; ++i)
	{
		pthread_t th;
		int res = pthread_create(&th, nullptr, &ThreadPool::_ThreadCallback, this);
		if(res) throw SystemException(res);

		m_threads.push_back(th);
	}
}

void ThreadPool::Stop()
{
	// Clreanup each thread object.
	std::for_each(std::begin(m_threads), std::end(m_threads),
		[](pthread_t& th) {pthread_join(th, nullptr);});

	// Clear since ther's no more active threads.
	m_threads.clear();
}

#endif // _WIN64