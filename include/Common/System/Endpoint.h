#if !defined (__ENDPOINT_H__)
#define __ENDPOINT_H__

#include "CommonDefinitions.h"

#ifdef _WIN64

// Interface of endpoint to be controlled by completion port.
struct IEndpoint
{
	virtual ~IEndpoint() = default;

	virtual bool IsValid() = 0;
	virtual SOCKET Get() = 0;
	virtual LPWSAOVERLAPPED GetContext() = 0;
	virtual void ResetContext() = 0;

	virtual void Complete(LPWSAOVERLAPPED context, ULONG dataTransferred) = 0;
};

template <typename Impl>
class CEndpoint : public IEndpoint
{
public:
	template<typename... Args>
	CEndpoint(Args&&... args)
	: m_impl(std::forward<Args>(args)...)
	{}

	virtual ~CEndpoint() = default;

	bool IsValid() override { return m_impl.IsValid(); }
	SOCKET Get() override { return m_impl.Get(); }
	LPWSAOVERLAPPED GetContext() override { return m_impl.GetContext(); }
	void ResetContext() override { m_impl.ResetContext(); }

	void Complete(LPWSAOVERLAPPED context, ULONG dataTransferred) override
	{
		m_impl.Complete(context, dataTransferred);
	}

protected:
	Impl m_impl;
};

static auto SocketDeleter = [](SOCKET x)->void {if (x) closesocket(x); };

template 
<
    typename Derived,
    typename HandleType = SOCKET,
    typename HandleDeleterType = decltype(SocketDeleter),
    typename ContextType = WSAOVERLAPPED
>
class CEndpointImplBase
{
protected:
	ContextType m_context;
	HandleType m_hEndpoint;
	HandleDeleterType& m_deleter;

	CRTP_SELF(Derived)

public:
	CEndpointImplBase(HandleDeleterType& deleter = SocketDeleter)
		: m_hEndpoint((HandleType)0)
		, m_deleter(deleter)
	{
		ResetContext();
	}

	~CEndpointImplBase()
	{
		m_deleter(m_hEndpoint);
	}

	bool IsValid()
	{
		return m_hEndpoint && (m_hEndpoint != (HandleType)~0);
	}

	HandleType Get()
	{
		return m_hEndpoint;
	}

	ContextType* GetContext()
	{
		return &m_context;
	}

	void ResetContext()
	{
		Self().ResetContext();
	}
};

class CAcceptorImpl final : public CEndpointImplBase<CAcceptorImpl>
{
    addrinfo* m_addrInfo;
	boost::function<void(void)> m_acceptCallback;
	// Buffer must be big enough to hold following info:
	// 1. The number of bytes reserved for the local address information.
	// 	  This value must be at least 16 bytes more than
	//    the maximum address length for the transport protocol in use.
	// 2. The number of bytes reserved for the remote address information.
	// 	  This value must be at least 16 bytes more than
	//    the maximum address length for the transport protocol in use.
	// See AcceptEx API docummentation for more details.
	boost::array<BYTE, (sizeof(SOCKADDR_IN6_PAIR) + 32)> m_acceptData;
	SOCKADDR_IN6* m_peerAddr;
	LPFN_ACCEPTEX m_pfnAcceptEx;
	LPFN_GETACCEPTEXSOCKADDRS m_pfnGetAcceptExSockaddrs;

	using Base_t = CEndpointImplBase<CAcceptorImpl>;

public:
    CAcceptorImpl(USHORT port, boost::function<void(void)> acceptCallback);
    ~CAcceptorImpl();

    void Complete(LPOVERLAPPED context, ULONG dataTransferred);
	void ResetContext();
	void Accept(IEndpoint* connection);
	std::string GetPeerInfo();
};

class CAcceptor : public CEndpoint<CAcceptorImpl>
{
	using Base_t = CEndpoint<CAcceptorImpl>;
public:
	CAcceptor(USHORT port, boost::function<void(void)> acceptCallback)
	: Base_t(port, acceptCallback) {}

	void Accept(IEndpoint* connection) {m_impl.Accept(connection);}
	std::string GetPeerInfo() {return m_impl.GetPeerInfo();}
};

#endif // _WIN64

#endif // __ENDPOINT_H__
