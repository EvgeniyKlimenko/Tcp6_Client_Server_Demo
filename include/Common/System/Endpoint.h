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

	virtual void Complete(ULONG dataTransferred) = 0;
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

	void Complete(ULONG dataTransferred) override
	{
		m_impl.Complete(dataTransferred);
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

using OperationCallback_t = boost::function<void (void)>;
class CAcceptorImpl final : public CEndpointImplBase<CAcceptorImpl>
{
    addrinfo* m_addrInfo;
	OperationCallback_t m_acceptCallback;
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
    CAcceptorImpl(USHORT port, OperationCallback_t&& acceptCallback);
    ~CAcceptorImpl();

    void Complete(ULONG dataTransferred);
	void ResetContext();
	void Accept(IEndpoint* connection);
	std::string GetPeerInfo();
};

class CConnectionImpl final :  public CEndpointImplBase<CAcceptorImpl>
{
	enum State
	{
		initial,
		readPending,
		writePending,
		disconnectPending	
	};

	using StateCallbackSequence_t = boost::unordered_map<State, OperationCallback_t>;

public:
	using Buffer_t = boost::asio::streambuf;

	CConnectionImpl(
		OperationCallback_t&& readCallback,
		OperationCallback_t&& writeCallback,
		OperationCallback_t&& disconnectCallback
		);

	~CConnectionImpl();

	void Read();
	void Write(const _tstring& data);
	_tstring GetInputData();

	void Complete(ULONG dataTransferred);
	void ResetContext();

private:
	void SwitchTo(State s) {m_curState = s;}
	void Disconnect();
	void Reset();

	static const size_t MAX_BUF_SIZE = 1024;
	State m_curState;
	Buffer_t m_buf;
	StateCallbackSequence_t m_callbacks;
	LPFN_DISCONNECTEX m_pfnDisconnectEx;
};

class CAcceptor final : public CEndpoint<CAcceptorImpl>
{
	using Base_t = CEndpoint<CAcceptorImpl>;
public:
	CAcceptor(USHORT port, OperationCallback_t&& acceptCallback)
	: Base_t(port, std::forward<OperationCallback_t>(acceptCallback)) {}

	void AcceptAsync(IEndpoint* connection) {m_impl.Accept(connection);}
	std::string GetPeerInfo() {return m_impl.GetPeerInfo();}
};

class CConnection final : public CEndpoint<CConnectionImpl>
{
	using Base_t = CEndpoint<CConnectionImpl>;
public:
	CConnection(
		OperationCallback_t&& readCallback,
		OperationCallback_t&& writeCallback,
		OperationCallback_t&& disconnectCallback)
	: Base_t(
		std::forward<OperationCallback_t>(readCallback),
		std::forward<OperationCallback_t>(writeCallback),
		std::forward<OperationCallback_t>(disconnectCallback)) {}

	void ReadAsync() {m_impl.Read();}
	void WriteAsync(const _tstring& data) {m_impl.Write(data);}
	_tstring GetInputData() {return m_impl.GetInputData();}
};

#endif // _WIN64

#endif // __ENDPOINT_H__
