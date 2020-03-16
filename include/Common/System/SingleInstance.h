#if !defined(__SINGLE_INSTANCE_H__)
#define __SINGLE_INSTANCE_H__

#include "CommonDefinitions.h"
#include "Exception.h"

template <bool Yes>
struct SingleInstance final {};

struct NullSingleInstanceImpl final {};

template <>
struct SingleInstance<false>
{
    NullSingleInstanceImpl m_impl;
};

#if defined(__linux__)

class SingleInstanceImpl final
{
    int m_fd;
    std::string m_path;

public:
    SingleInstanceImpl()
    {
        std::stringstream path;
        path << "/tmp/" << program_invocation_short_name <<".pid";
        m_path = path.str();
        m_fd = open(m_path.c_str(), O_CREAT | O_RDONLY);
        if (m_fd < 0) throw SystemException(errno);

        int res = flock(m_fd, LOCK_EX | LOCK_NB);
        if (res < 0)
        {
            // Close handle before throwing exception to eliminate resource leak.
            close(m_fd);
            // Zero out file handle to prevent removing it by dtor.
            m_fd = 0;
			// Second instance, throw exception.
			std::stringstream msg;
			msg << "Only one instance of " << program_invocation_short_name << " allowed in the same time.";
			throw std::logic_error(msg.str());
        }
    }

    ~SingleInstanceImpl()
    {
        if (m_fd)
        {
            close(m_fd);
            remove(m_path.c_str());
        }
    }
};

template <>
struct SingleInstance<true>
{
    SingleInstanceImpl m_impl;
};

#elif defined(_WIN64)

class CSingleInstanceImpl final
{
	HANDLE m_hMutex;

public:
	CSingleInstanceImpl()
	{
        // Getting binary name before naming mutex.
        _tstring exeName;
        exeName.resize(MAX_PATH);
        LPTSTR lpExeName = const_cast<LPTSTR>(exeName.c_str());
        ULONG res = GetModuleFileName(nullptr, lpExeName, MAX_PATH);
        if (!res) throw CWindowsException(GetLastError());
        exeName.resize(res);

        // Only file name needed.
        size_t pos = exeName.find_last_of(_T("\\"));
        if(pos++ != _tstring::npos) exeName = exeName.substr(pos);
        LPCTSTR lpcExeName = exeName.c_str();

        // Mutex will be named the same as binary file.
		m_hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, lpcExeName);
		if (m_hMutex)
		{
			// Close handle before throwing exception to eliminate resource leak.
			CloseHandle(m_hMutex);
			// Second instance, throw exception.
			_tstringstream tssMsg;
			tssMsg << _T("Only one instance of ") << exeName << _T(" allowed in the same time.");
			_tstring tsMsg = tssMsg.str();
			throw std::logic_error(std::string(tsMsg.begin(), tsMsg.end()));
		}
		else
		{
			m_hMutex = CreateMutex(nullptr, FALSE, lpcExeName);
		}
	}

	~CSingleInstanceImpl()
	{
		if (m_hMutex)
			CloseHandle(m_hMutex);
	}
};

template <>
struct SingleInstance<true>
{
    CSingleInstanceImpl m_impl;
};

#endif // __linux__

#endif // __SINGLE_INSTANCE_H__