#if !defined( __COMMON_DEFINITIONS_H__ )
#define __COMMON_DEFINITIONS_H__

#if defined ( __linux__ )

#include <sys/sysinfo.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

#elif defined ( _WIN64 )

#pragma warning(disable:4244)

#include <windows.h>
#include <tchar.h>
#include <process.h>

#endif // __linux__

#include <string>
#include <sstream>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <queue>
#include <list>
#include <array>
#include <algorithm>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cassert>

#include <boost/exception/all.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/function.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/program_options.hpp>

#if defined ( _WIN64 )

using _tstring = std::basic_string<_TCHAR>;
using _tstringstream = std::basic_stringstream<_TCHAR>;

#endif // _WIN64

#define CRTP_SELF(Target) \
    Target& Self() { return static_cast<Target&>(*this); }

#endif // __COMMON_DEFINITIONS_H__