#if !defined(__SERVER_H__)
#define __SERVER_H__

#include "AppLogic.h"
#include "System/WinSockIniter.h"
#include "System/Endpoint.h"
#include "System/IoManager.h"
#include "System/ThreadPool.h"
#include "System/Synchronization.h"

class AsioServer final : public AppLogic<AsioServer, true>
{
class Connection : public boost::enable_shared_from_this<Connection>
{
    static const size_t BUF_SIZE = 1024;
    boost::asio::ip::tcp::socket m_sock;
    char m_data[BUF_SIZE];
    boost::function<void(const boost::system::error_code& err, size_t bytesRead)> m_readCallback;
    boost::function<void(const boost::system::error_code& err, size_t bytesRead)> m_writeCallback;

    Connection(boost::asio::io_service& ioSrv)
    : m_sock(ioSrv)
    {
        memset(m_data, 0, BUF_SIZE);
    }

public:
    using Pointer_t = boost::shared_ptr<Connection>;

    static Pointer_t Create(boost::asio::io_service& ioSrv)
    {
        return Pointer_t(new Connection(ioSrv));
    }

    boost::asio::ip::tcp::socket& GetSocket()
    {
        return m_sock;
    }

    void Start();
    void OnReadComplete(const boost::system::error_code& err, size_t bytesRead);
    void OnWriteComplete(const boost::system::error_code& err, size_t bytesWritten);
};

public:
    AsioServer(uint16_t port);
    ~AsioServer() { Stop(); }

    void OnRun();
    void OnStop();

private:
    void ThreadCallback();
    void StartListening();
    void OnAccept(Connection::Pointer_t connection, const boost::system::error_code& err);

private:
    boost::asio::io_service m_ioSvc;
    boost::asio::ip::tcp::endpoint m_endpoint;
    boost::asio::ip::tcp::acceptor m_acceptor;
    boost::thread_group m_threadPool;
};

template
<
    typename Derived,
    typename Acceptor,
    typename ThreadPool,
    typename IoManager,
    typename Connection,
    typename ConnectionManager,
    typename SocketSubsystemIniter
>
class SystemServer : public AppLogic
<
    SystemServer
    <
        Derived, Acceptor, ThreadPool, IoManager,
        Connection, ConnectionManager, SocketSubsystemIniter
    >, true
>
{
    CRTP_SELF(Derived)
public:
    SystemServer(unsigned short port)
    : m_port(port)
    , m_acceptor(m_port, boost::bind(&SystemServer::OnAcceptComplete, this, _1))
    , m_threadPool(boost::bind(&SystemServer::AsyncWorkCallback, this))
    , m_ioMgr(m_threadPool.GetThreadCount())
    , m_cnMgr(boost::bind(&SystemServer::CreateConnection, this)) 
    {
        m_ioMgr.Bind(&m_acceptor);
    }

    ~SystemServer()
    {
        // GCC doesn't see member function of the base template class.
        this->Stop();
    }

    void OnRun()
    {
        DoAccept();
        m_threadPool.Start();

        std::cout << "System API based server ::1(" << m_port << ") is ready." << std::endl;
        std::cout << "Press any key to exit." << std::endl;
        std::cin.get();
    }

    void OnStop()
    {
        std::cout << "Finishing system API based server..." << std::endl;
        m_ioMgr.Stop();
        m_threadPool.Stop();
        std::cout << "System API based server finished." << std::endl;
    }

    IConnection* CreateConnection()
    {
        return Self().CreateConnection();
    }

    bool DoAccept()
    {
        return m_acceptor.AcceptAsync(m_cnMgr.Get());
    }

    void OnAcceptComplete(IConnection* newConnection)
    {
        // Print new peer.
        std::cout << m_acceptor.GetPeerInfo() << std::endl;
        // Start tracking next connection.
        if (DoAccept())
        {
            // Start read IO on new connection.
            newConnection->ReadAsync();
        }
    }

protected:
    void AsyncWorkCallback()
    {
        try
        {
            m_ioMgr.Run();
        }
        catch(...)
        {
            // GCC doesn't see member variable of the base template class.
            this->m_exceptioning.Append(boost::current_exception());
        }
    }

protected:
    unsigned short m_port;
    SocketSubsystemIniter m_sockIniter;
    Acceptor m_acceptor;
    ThreadPool m_threadPool;
    IoManager m_ioMgr;
    ConnectionManager m_cnMgr;
};

#if defined(USE_NATIVE)

#if defined(_WIN64)

class CWinSockServer final : public SystemServer
<
    CWinSockServer,
    CAcceptor,
    CThreadPool,
    CIoManager,
    CConnection,
    ConnectionManager
    < 
        1, PointerList_t,
        boost::function<IConnection* (void)>,
        CWindowsLock, ScopedLocker
    >,
    CWinSockIniter
>
{
public:
    CWinSockServer(unsigned short port)
    : SystemServer(port) {}

    IConnection* CreateConnection();

private:
    void OnReadComplete(IConnection* connection);
    void OnWriteComplete(IConnection* connection);
    void OnDisconnectComplete(IConnection* connection);
};

using CurrentServer = CWinSockServer;

#elif defined(__linux__)

// A stub for socket subsystem initialization, 
// corresponds to null object design pattern.
// Unlike Windows, Linux socket subsystem doesn't
// requere special initialization.
struct SubsysIniterNullObj {};

class LinuxServer final : public SystemServer
<
    LinuxServer,
    Acceptor,
    ThreadPool,
    IoManager,
    Connection,
    ConnectionManager
    < 
        1, PointerList_t,
        boost::function<IConnection* (void)>,
        LinuxLock, ScopedLocker
    >,
    SubsysIniterNullObj
>
{
public:
    LinuxServer(unsigned short port)
    : SystemServer(port) {}

    IConnection* CreateConnection();

private:
    size_t OnReadComplete(IConnection* connection);
    size_t OnWriteComplete(IConnection* connection);
};

using CurrentServer = LinuxServer;

#endif // _WIN64

#else

using CurrentServer = AsioServer;

#endif // USE_NATIVE

#endif // __SERVER_H__