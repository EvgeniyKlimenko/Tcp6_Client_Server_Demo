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

#if defined (__linux__)

// TODO

#elif defined(_WIN64)

class CWinSockServer final : public AppLogic<CWinSockServer, true>
{
    using ConnectionManager_t = ConnectionManager
    < 
        5, IConnection, std::list<IConnection*>,
        boost::function<IConnection* (void)>,
        CWindowsLock, ScopedLocker
    >;
public:
    CWinSockServer(USHORT port);
    ~CWinSockServer();

    void OnRun();
    void OnStop();

private:
    void AsyncWorkCallback();
    void OnAcceptComplete(IConnection* newConnection);
    void OnReadComplete(IConnection* connection);
    void OnWriteComplete(IConnection* connection);
    void OnDisconnectComplete(IConnection* connection);
    IConnection* CreateConnection();

    void DoAccept();

private:
    CWinSockIniter m_wsIniter;
    CAcceptor m_acceptor;
    CThreadPool m_threadPool;
    CIoManager m_ioMgr;
    ConnectionManager_t m_cnMgr;
};

#endif

#if defined USE_NATIVE
using CurrentServer = CWinSockServer;
#else
using CurrentServer = AsioServer;
#endif // USE_NATIVE

#endif // _SERVER_H__