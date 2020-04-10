#if !defined( __COMMON_DEFINITIONS_H__ )
#define __COMMON_DEFINITIONS_H__

#if defined ( __linux__ )

#include <sys/sysinfo.h>
#include <sys/file.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>

extern char* program_invocation_name;
extern char* program_invocation_short_name;

#elif defined ( _WIN64 )

#pragma warning(disable:4244)
#pragma warning(disable:4834)

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define BOOST_ASIO_WINDOWS
#define _WINSOCK_DEPRECATED_NO_WARNINGS	// Exclude old style socket API warnings
#define _SECURE_SOCKET_TYPES_DEFINED_	// Include IPSec extensions API

#include <windows.h>
#include <winsock2.h>
#include <tchar.h>
#include <process.h>
#include <mstcpip.h>
#include <WS2tcpip.h>
#include <MSWSock.h>

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
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/array.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#define USE_NATIVE

#if defined ( _WIN64 )

using _tstring = std::basic_string<_TCHAR>;
using _tstringstream = std::basic_stringstream<_TCHAR>;

#endif // _WIN64

#define CRTP_SELF(Target) \
    Target& Self() { return static_cast<Target&>(*this); }

#endif // __COMMON_DEFINITIONS_H__