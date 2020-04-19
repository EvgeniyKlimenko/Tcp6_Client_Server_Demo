#include "System/TerminationLogic.h"

#if defined (_WIN64)

CWindowsTerminationLogic* CWindowsTerminationLogic::s_self;

#elif defined(__linux__)

LinuxTerminationLogic* LinuxTerminationLogic::s_self;

#endif // _WIN64

