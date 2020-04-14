#if !defined(__SYNCHRONIZATION_H__)
#define __SYNCHRONIZATION_H__

#include "CommonDefinitions.h"

template < typename DerivedType > class GenericLock
{
public:
    GenericLock() = default;
    ~GenericLock() = default;

    void Lock ()
    {
        Self().Lock();
    }

    void Unlock ()
    {
        Self().Unlock();
    }

protected:
    DerivedType & Self()
    {
        return (static_cast < DerivedType & >(*this));
    }
};

template < typename LockType > class ScopedLocker
{
    LockType& m_lock;
public:    
    ScopedLocker(LockType& lock) : m_lock(lock) { m_lock.Lock(); }
    ~ScopedLocker() { m_lock.Unlock(); }
};

#if defined(_WIN64)

class CWindowsLock : public GenericLock<CWindowsLock>
{
    CRITICAL_SECTION m_cs;
public:
    CWindowsLock()
    {
        InitializeCriticalSection(&m_cs);
    }
    
    ~CWindowsLock()
    {
        DeleteCriticalSection(&m_cs);
    }

    void Lock()
    {
        EnterCriticalSection(&m_cs);
    }

    void Unlock()
    {
        LeaveCriticalSection(&m_cs);
    }
};

#elif defined(__linux__)

class LinuxLock : public GenericLock<LinuxLock>
{
    pthread_mutex_t m_mtx;
public:
    LinuxLock()
    {
        pthread_mutex_init(&m_mtx, nullptr);
    }
    
    ~LinuxLock()
    {
        pthread_mutex_destroy(&m_mtx);
    }

    void Lock()
    {
        pthread_mutex_lock(&m_mtx);
    }

    void Unlock()
    {
        pthread_mutex_unlock(&m_mtx);
    }
};

#endif // _WIN64

#endif // __SYNCHRONIZATION_H__