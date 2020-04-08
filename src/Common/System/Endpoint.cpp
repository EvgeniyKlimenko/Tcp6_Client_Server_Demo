#include "System/Endpoint.h"
#include "System/Exception.h"

#ifdef _WIN64

 CAcceptorImpl::CAcceptorImpl(USHORT port, boost::function<void(void)> acceptCallback)
 : m_addrInfo(nullptr)
 , m_acceptCallback(acceptCallback)
 , m_peerAddr(nullptr)
 {
     // Create acceptor endpoint.
     m_hEndpoint = WSASocket(AF_INET6, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (m_hEndpoint == INVALID_SOCKET) throw CWindowsException(::WSAGetLastError());
    
	// Obtain advanced socket API supporting IO completion port principle.

	// AcceptEx API needed to accept peer connections asynchronously.
	GUID funcId = WSAID_ACCEPTEX;
	ULONG bytesRet = 0;

	if (WSAIoctl(m_hEndpoint, SIO_GET_EXTENSION_FUNCTION_POINTER, &funcId, sizeof(GUID),
		&m_pfnAcceptEx, sizeof(PVOID), &bytesRet, nullptr, nullptr) == SOCKET_ERROR)
	{
		throw CWindowsException(WSAGetLastError());
	}

	// GetAcceptExSockaddrs needed to obtain peer information.
	funcId = WSAID_GETACCEPTEXSOCKADDRS;
	bytesRet = 0;

	if (WSAIoctl(m_hEndpoint, SIO_GET_EXTENSION_FUNCTION_POINTER, &funcId, sizeof(GUID),
		&m_pfnGetAcceptExSockaddrs, sizeof(PVOID), &bytesRet, nullptr, nullptr) == SOCKET_ERROR)
	{
		throw CWindowsException(WSAGetLastError());
	}

	/*funcId = WSAID_DISCONNECTEX;
	bytesRet = 0;

	if (::WSAIoctl(m_sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &funcId, sizeof(GUID),
		&m_pfnDisconnectEx, sizeof(PVOID), &bytesRet, nullptr, nullptr) == SOCKET_ERROR)
	{
		throw CWindowsException(::WSAGetLastError());
	}*/

	// Reuse server address to get read of possible errors the previous connection has not been fully disconnected.
	BOOL reuseAddr = true;
	if (setsockopt(m_hEndpoint, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseAddr, sizeof(BOOL)) == SOCKET_ERROR)
		throw CWindowsException(WSAGetLastError());

	// Initialize IPv6 address data.
    addrinfo hint = {};
    hint.ai_flags = AI_PASSIVE | AI_NUMERICHOST;
    hint.ai_family = AF_INET6;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_protocol = IPPROTO_TCP;

	std::stringstream portHint;
	portHint << port;

    int res = getaddrinfo("::1", portHint.str().c_str(), &hint, &m_addrInfo);
    if(res) throw CWindowsException(res);

	// Bind endpoint to the IPv6 address.
    if (bind(m_hEndpoint, m_addrInfo->ai_addr, static_cast<int>(m_addrInfo->ai_addrlen)) == SOCKET_ERROR)
        throw CWindowsException(WSAGetLastError());

	// Start listening to peer connections.
    if (listen(m_hEndpoint, 1) == SOCKET_ERROR)
		throw CWindowsException(WSAGetLastError());
 }

 CAcceptorImpl::~CAcceptorImpl()
 {
     if(m_addrInfo)
        freeaddrinfo(m_addrInfo);
 }

void CAcceptorImpl::Complete(LPOVERLAPPED, ULONG)
{
	// This callback triggded only when a new connection accepted.
	// In this case a callback from the server called.
	// We use server callback because accept operation should be handled
	// by the whole server, not an acceptor only. Thereby only the acceptor
	// is able to track accpet operation completion.
	m_acceptCallback();
}

void CAcceptorImpl::ResetContext()
{
	memset(&m_context, 0, sizeof(WSAOVERLAPPED));
	std::fill(std::begin(m_acceptData), std::end(m_acceptData), 0);
}

void CAcceptorImpl::Accept(IEndpoint* connection)
{
	PBYTE acceptBuf = &m_acceptData[0];
	ULONG addrLen = static_cast<ULONG>(sizeof(SOCKADDR_IN6) + 16);
	if (!m_pfnAcceptEx(m_hEndpoint, connection->Get(), acceptBuf, 0,
		addrLen, addrLen, nullptr, &m_context))
	{
		ULONG err = WSAGetLastError();
		if (err != ERROR_IO_PENDING) throw CWindowsException(err);
	}
}

std::string CAcceptorImpl::GetPeerInfo()
{
	// Client has been just connected.

	PBYTE acceptBuf = &m_acceptData[0];
	ULONG addrLen = static_cast<ULONG>(sizeof(SOCKADDR_IN6) + 16);
	SOCKADDR_IN6* localAddr = nullptr;
	int localAddrLen = 0;
	int peerAddrLen = 0;

	// Retain the peer to be inspected later.

	m_pfnGetAcceptExSockaddrs(acceptBuf, 0, addrLen, addrLen, 
		reinterpret_cast<LPSOCKADDR*>(&localAddr), &localAddrLen,
		reinterpret_cast<LPSOCKADDR*>(&m_peerAddr), &peerAddrLen);

	std::string hostName;
	std::string serviceName;

	hostName.resize(NI_MAXHOST);
	serviceName.resize(NI_MAXSERV);

	// Print peer data.

	if (getnameinfo(reinterpret_cast<LPSOCKADDR>(m_peerAddr), peerAddrLen,
		const_cast<char*>(hostName.c_str()), static_cast<ULONG>(hostName.length()),
		const_cast<char*>(serviceName.c_str()), static_cast<ULONG>(serviceName.length()), 0) == SOCKET_ERROR)
	{
		throw CWindowsException(WSAGetLastError());
	}

	hostName.resize(strlen(hostName.c_str()));
	serviceName.resize(strlen(serviceName.c_str()));

	std::stringstream peerInfo;
	peerInfo << "Peer " << hostName << ":" << serviceName << " connected to the endpoint " << std::setbase(std::ios::hex) << this << ".";
	return peerInfo.str();
}

#endif