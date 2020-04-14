#if !defined(__EXCEPTION_H__)
#define __EXCEPTION_H__

#include "CommonDefinitions.h"

class SystemException final : public std::exception
{
    int m_err;
public:
    SystemException(int err);
    SystemException(const SystemException& other);
    SystemException& operator= (const SystemException& other);
    virtual ~SystemException () = default;
    
    const char* what()  const noexcept override;

	static std::string GetErrorDescription(int err);
};

template <typename Exception>
class ExceptionDecorator final : public std::exception
{
	Exception m_ex;
public:
	ExceptionDecorator& operator= (const ExceptionDecorator& other)
	{
		if (this != &other) m_ex = other.m_ex;
		return *this;
	}
	virtual ~ExceptionDecorator() = default;

	const char* what() const noexcept override { return m_ex.what(); }

	static std::string GetErrorDescription();
};

#if defined(_WIN64)
class CWindowsException final : public std::exception
{
public:
	CWindowsException(ULONG code) : m_code(code)
	{
		ResolveDescription();
	}

	CWindowsException(const CWindowsException& other);
	virtual ~CWindowsException() = default;

	CWindowsException& operator = (const CWindowsException& other);
	const char* what() const override;

	static std::string GetErrorDescription(ULONG code);

private:
	void ResolveDescription();

private:
	ULONG m_code;
	std::string m_description;
};

// SEH exceptions. Establishes own exception handler
// to translate SEH to C++ exceptions
// and throw them anew as C++ exceptions
class CSehException final : public std::exception
{
public:
	static void Setup();
	static void Remove();

	CSehException(const CSehException& other);
	CSehException& operator = (const CSehException& other);
	virtual ~CSehException() = default;

	const char* what() const override;

private:
	static void Handler(unsigned int code, PEXCEPTION_POINTERS extraData);

	CSehException(unsigned int code, PEXCEPTION_POINTERS extraData);

	void ResolveDescription();

private:
	static _se_translator_function s_prev;
	unsigned int m_code;
	PVOID m_addr;
	std::string m_description;
};

template <>
class ExceptionDecorator<CWindowsException> final
{
	CWindowsException m_ex;
public:
	ExceptionDecorator() : m_ex(WSAGetLastError()) {}
	ExceptionDecorator(int e) : m_ex((int)e) {}

	static std::string GetErrorDescription()
	{
		return CWindowsException::GetErrorDescription(WSAGetLastError());
	}
};

using CWinSockExceptionDecorator =  ExceptionDecorator<CWindowsException>;

#elif defined(__linux__)

template <>
class ExceptionDecorator<SystemException> final
{
	SystemException m_ex;
public:
	ExceptionDecorator() : m_ex(errno) {}
	ExceptionDecorator(int e) : m_ex(e) {}

	static std::string GetErrorDescription()
	{
		return SystemException::GetErrorDescription(errno);
	}
};

using SystemExceptionDecorator =  ExceptionDecorator<SystemException>;

#endif // _WIN64

#endif // __EXCEPTION_H__