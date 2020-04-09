#if !defined(__THREAD_POOL_H__)
#define __THREAD_POOL_H__

#include "CommonDefinitions.h"

#ifdef _WIN64

class CThreadPool final
{
public:
    using ThreadCallback_t = boost::function<void (void)>;

    CThreadPool(ThreadCallback_t threadCallback);
    ~CThreadPool();

    size_t GetThreadCount() const;

    void Start();
    void Stop();

private:
    static unsigned int __stdcall _ThreadCallback(void* param);

private:
    ThreadCallback_t m_threadCallback;
    ULONG m_threadCount;
    std::vector<HANDLE> m_threads;
    
};

#endif // _WIN64

#endif // __THREAD_POOL_H__