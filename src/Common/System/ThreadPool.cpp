#include "System/ThreadPool.h"
#include "System/Exception.h"

#if defined(_WIN64)

CThreadPool::CThreadPool(CThreadPool::ThreadCallback_t threadCallback)
{
	// Determine number of dedicated threads.
	SYSTEM_INFO sysInfo = { 0 };
	GetSystemInfo(&sysInfo);
	ULONG threadCount = 2 * sysInfo.dwNumberOfProcessors + 1;

	// Create dedicated threads using VC++ runtime call. 
	for (unsigned i = 0; i < threadCount; ++i)
	{
		HANDLE th = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, 
			&CThreadPool::_ThreadCallback, this, 0, nullptr));
		if (!th) throw SystemException(errno);

		m_threads.push_back(th);
	}
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

void CThreadPool::Stop()
{
	// Without active working threads has no sence. 
	if(m_threads.empty()) return;

	// Wait while the dedicated threads finish.
	PHANDLE threadHandles = &m_threads[0];
	ULONG threadCount = static_cast<ULONG>(m_threads.size());
	WaitForMultipleObjects(threadCount, threadHandles, TRUE, INFINITE);

	// Clreanup each thread object.
	std::for_each(std::begin(m_threads), std::end(m_threads),
		[](HANDLE th) {CloseHandle(th);});
}

#endif // _WIN64