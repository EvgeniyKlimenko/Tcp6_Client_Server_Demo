#if !defined(__THREAD_POOL_H__)
#define __THREAD_POOL_H__

#include "CommonDefinitions.h"

#if defined(_WIN64)

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

#elif defined(__linux__)

class ThreadPool final
{
public:
    using ThreadCallback_t = boost::function<void (void)>;

    ThreadPool(ThreadCallback_t threadCallback);
    ~ThreadPool();

    size_t GetThreadCount() const;

    void Start();
    void Stop();

private:
    static void* _ThreadCallback(void* param);

private:
    ThreadCallback_t m_threadCallback;
    uint32_t m_threadCount;
    std::vector<pthread_t> m_threads;
    
};

#endif // _WIN64

#endif // __THREAD_POOL_H__