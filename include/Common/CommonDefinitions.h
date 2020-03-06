#if !defined( __COMMON_DEFINITIONS_H__ )
#define __COMMON_DEFINITIONS_H__

#if defined ( __linux__ )

#include <sys/sysinfo.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

#elif defined ( _WIN64 )

#include <windows.h>
#include <tchar.h>
#include <process.h>

#endif // __linux__

#include <string>
#include <sstream>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <exception>
#include <stdexcept>
#include <queue>
#include <list>
#include <array>
#include <memory>
#include <algorithm>
#include <functional>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cassert>

#if defined ( _WIN64 )

using _tstring = std::basic_string<_TCHAR>;
using _tstringstream = std::basic_stringstream<_TCHAR>;

#endif // _WIN64

#endif // __COMMON_DEFINITIONS_H__