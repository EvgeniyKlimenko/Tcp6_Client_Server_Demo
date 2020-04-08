#include "System/IoManager.h"
#include "System/Exception.h"

#if defined(_WIN64)

CIoManager::CIoManager(size_t threadCount)
: m_threadCount(threadCount)
{
	// Create empty completion port object.
	m_hPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	if (!m_hPort)
		throw CWindowsException(GetLastError());
}
	
CIoManager::~CIoManager()
{
	Stop();
	CloseHandle(m_hPort);
}

void CIoManager::Bind(IEndpoint* pEndpoint)
{
	if(!pEndpoint || !pEndpoint->IsValid())
		throw CWindowsException(ERROR_INVALID_PARAMETER);

	// Associate endpoint with a completion port.
	if(!CreateIoCompletionPort(reinterpret_cast<HANDLE>(pEndpoint->Get()), m_hPort,
		reinterpret_cast<ULONG_PTR>(pEndpoint), 0))
	{
		throw CWindowsException(GetLastError());
	}
}

void CIoManager::Stop()
{
	// Send finish to all threads bound to the port.
	// The last parameter is context of null value.
	for(ULONG i = 0; i < m_threadCount; ++i)
		PostQueuedCompletionStatus(m_hPort, 0, 0, nullptr);
}
	
void CIoManager::Run()
{
	// Set SEH exception wrapper.
	CSehException::Setup();

	for (;;)
	{
		ULONG bytesTransferred = 0;
		IEndpoint* pEndpoint = nullptr;
		LPOVERLAPPED pContext = nullptr;

		// Start processing of the next data portion.
		if (!GetQueuedCompletionStatus(m_hPort, &bytesTransferred, reinterpret_cast<PULONG_PTR>(&pEndpoint),
			&pContext, INFINITE))
		{
			throw CWindowsException(GetLastError());
		}

		// A null context passed by finishing.
		if (!pContext)
			break;

		// Data arrived for certain endpoint. Each endpoint is aware how to complete this data portion.
		pEndpoint->Complete(pContext, bytesTransferred);
	}
}

#endif // _WIN64