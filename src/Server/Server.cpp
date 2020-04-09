#include "CommonDefinitions.h"
#include "Server.h"

void AsioServer::Connection::Start()
{
    auto self = m_sock.local_endpoint();
    auto peer = m_sock.remote_endpoint();
    std::cout << "Server " << self.address().to_string() << "(" 
        << self.port() << ") accepted client " 
        << peer.address().to_string() 
        << "(" << peer.port() << ")." << std::endl;

    if(!m_readCallback)
    {
        m_readCallback = boost::bind(
            &AsioServer::Connection::OnReadComplete,
            shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred);
    }

    if(!m_writeCallback)
    {
        m_writeCallback = boost::bind(
                &AsioServer::Connection::OnWriteComplete,
                shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred);
    }

    // Start asynchronous reading so we're waiting messages from client.
    m_sock.async_read_some(boost::asio::buffer(m_data, BUF_SIZE), m_readCallback);
}

void AsioServer::Connection::OnReadComplete(const boost::system::error_code& err, size_t bytesRead)
{
    if(err)
    {
        std::cerr << "Error reading data: " << err.message() << std::endl;
        m_sock.close();
    }
    else
    {
        // Got message from a client.
        std::string dataReceived(m_data, bytesRead);
        std::cout << "Data received: " << dataReceived << std::endl;

        // Write this message back to the client.
        m_sock.async_write_some(boost::asio::buffer(m_data, BUF_SIZE), m_writeCallback);
    }
    
}

void AsioServer::Connection::OnWriteComplete(const boost::system::error_code& err, size_t)
{
    if(err)
    {
        std::cerr << "Error writing data: " << err.message() << std::endl;
        m_sock.close();
    }
    else
    {
        std::cout << "Echo message has been sent." << std::endl;

        // After sending echo to the client we're waiting for more messages.
        m_sock.async_read_some(boost::asio::buffer(m_data, BUF_SIZE), m_readCallback);
    }
}

AsioServer::AsioServer(uint16_t port)
: m_endpoint(boost::asio::ip::tcp::v6(), port)
, m_acceptor(m_ioSvc, m_endpoint)
{}

void AsioServer::OnRun()
{
    StartListening();

    uint32_t threadNum = boost::thread::hardware_concurrency();
    while(threadNum--)
        m_threadPool.create_thread(boost::bind(&AsioServer::ThreadCallback, this));

    std::cout << "Server " << m_endpoint.address().to_string() << "(" << m_endpoint.port() << ") is ready." << std::endl;
    std::cout << "Press any key to exit." << std::endl;
    std::cin.get();
}

void AsioServer::OnStop()
{
    std::cout << "Finishing ASIO server..." << std::endl;
    m_ioSvc.stop();
    m_acceptor.close();
    m_threadPool.join_all();
    std::cout << "Server ASIO finished." << std::endl;
}

void AsioServer::ThreadCallback()
{
    try
    {
        m_ioSvc.run();
    }
    catch(...)
    {
        m_exceptioning.Append(boost::current_exception());
    }
}

void AsioServer::StartListening()
{
    Connection::Pointer_t connection = Connection::Create(m_ioSvc);
    m_acceptor.async_accept(
        connection->GetSocket(),
        boost::bind(
            &AsioServer::OnAccept,
            this,
            connection,
            boost::asio::placeholders::error));
}

void AsioServer::OnAccept(AsioServer::Connection::Pointer_t connection, const boost::system::error_code& err)
{
    if (!err) connection->Start();
    StartListening();
}

#if defined(_WIN64)

CWinSockServer::CWinSockServer(USHORT port)
: m_acceptor(port, boost::bind(&CWinSockServer::OnAcceptComplete, this, _1))
, m_threadPool(boost::bind(&CWinSockServer::AsyncWorkCallback, this))
, m_ioMgr(m_threadPool.GetThreadCount())
, m_cnMgr(boost::bind(&CWinSockServer::CreateConnection, this))
{
    m_ioMgr.Bind(&m_acceptor);
}

CWinSockServer::~CWinSockServer()
{
    Stop();
}

void CWinSockServer::OnRun()
{
    DoAccept();
    m_threadPool.Start();
}

void CWinSockServer::OnStop()
{
    std::cout << "Finishing Winsock server..." << std::endl;
    m_ioMgr.Stop();
    m_threadPool.Stop();
    std::cout << "Winsock server finished." << std::endl;
}

void CWinSockServer::AsyncWorkCallback()
{
    try
    {
        m_ioMgr.Run();
    }
    catch(...)
    {
        m_exceptioning.Append(boost::current_exception());
    }
}

void CWinSockServer::OnAcceptComplete(IConnection* newConnection)
{
    // Print new peer.
    std::cout << m_acceptor.GetPeerInfo() << std::endl;
    // Start tracking next connection.
    DoAccept();
    // Start read IO on new connection.
    newConnection->ReadAsync();
}

void CWinSockServer::OnReadComplete(IConnection* connection)
{
    // Asynchronous data reading just completed - get the data.
    _tstring data = connection->GetInputData();
    std::cout << "Data coming from peer: " << std::string(std::begin(data), std::end(data)) << std::endl;
    // Write the back back to the peer.
    connection->WriteAsync(data);
}

void CWinSockServer::OnWriteComplete(IConnection* connection)
{
    // Asynchronous data writing just completed - start reading new portion.
    connection->ReadAsync();
}

void CWinSockServer::OnDisconnectComplete(IConnection* connection)
{
    // Connection has been closed by peer - release it and prepare for reuse.
    m_cnMgr.Release(connection); 
}

IConnection* CWinSockServer::CreateConnection()
{
    IConnection* connection = new (std::nothrow) CConnection(
        boost::bind(&CWinSockServer::OnReadComplete, this, _1),
        boost::bind(&CWinSockServer::OnWriteComplete, this, _1),
        boost::bind(&CWinSockServer::OnDisconnectComplete, this, _1));
    if (!connection) throw std::bad_alloc();

    // Associate newly created connection with IO completion port
    // so that it's ready to asynchronous IO just now. 
    m_ioMgr.Bind(connection);

    return connection;
}

void CWinSockServer::DoAccept()
{
    m_acceptor.AcceptAsync(m_cnMgr.Get());
}

#endif // _WIN64