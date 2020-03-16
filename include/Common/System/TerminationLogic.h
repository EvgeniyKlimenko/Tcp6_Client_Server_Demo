#if !defined(__THERMINATION_LOGIC_H__)
#define __THERMINATION_LOGIC_H__

#include "CommonDefinitions.h"
#include "Exception.h"

using TerminationCallback_t = boost::function<void(void)>;

#if defined (__linux__)

class LinuxTerminationLogic final
{
    TerminationCallback_t m_callback;

    static void SignalCallback(int sig, siginfo_t* info, void* param)
    {
        LinuxTerminationLogic* handler = static_cast<LinuxTerminationLogic*>(param);
        std::cerr << "Termination callback for signal " << sig << ", process ID: " << info->si_pid << ", user ID: " << info->si_uid << "." << std::endl;
        handler->m_callback();
    }

public:
    LinuxTerminationLogic(TerminationCallback_t&& callback)
    : m_callback(callback)
    {
        struct sigaction sa;

        memset(&sa, 0, sizeof(sa));

        sigemptyset(&sa.sa_mask);
        sa.sa_sigaction = &LinuxTerminationLogic::SignalCallback;
        sa.sa_flags = SA_RESTART | SA_SIGINFO;
        
        std::array<int, 8> signumArray; 

        signumArray[0] = SIGALRM;
        signumArray[1] = SIGHUP;
        signumArray[2] = SIGINT;
        signumArray[3] = SIGQUIT;
        signumArray[4] = SIGTSTP;
        signumArray[5] = SIGTERM;
		signumArray[6] = SIGUSR1;
		signumArray[7] = SIGUSR2;

        std::for_each(std::begin(signumArray), std::end(signumArray), [&sa](int signum)
        {
            if(sigaction(signum, &sa, nullptr) < 0)
                throw SystemException(errno);
        });
    }
};

using DefaultTerminationLogic = LinuxTerminationLogic;

#elif defined (_WIN64)

class CWindowsTerminationLogic final
{
    static CWindowsTerminationLogic* s_self;
    TerminationCallback_t m_callback;

    static BOOL __stdcall ConsoleCtrlCallback(ULONG code)
    {
        std::cerr << "Console control callback for code " << code << " raised." << std::endl;
        s_self->m_callback();
        if (code ==  CTRL_C_EVENT || code ==  CTRL_BREAK_EVENT)
            ExitProcess(0);
        return TRUE;
    }

public:
    CWindowsTerminationLogic(TerminationCallback_t&& callback)
    : m_callback(callback)
    {
        if (!SetConsoleCtrlHandler(&CWindowsTerminationLogic::ConsoleCtrlCallback, TRUE))
            throw CWindowsException(GetLastError());

        CWindowsTerminationLogic::s_self = this;
    }
};

using DefaultTerminationLogic = CWindowsTerminationLogic;

#else

struct NullTerminationLogic final
{
    template<typename... Anything>
    NullTerminationLogic() {}
};

using DefaultTerminationLogic = NullTerminationLogic;

#endif // __linux__

#endif // __THERMINATION_LOGIC_H__