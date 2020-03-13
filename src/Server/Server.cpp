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
    std::cout << "Finishing server..." << std::endl;
    m_ioSvc.stop();
    m_acceptor.close();
    m_threadPool.join_all();
    std::cout << "Server finished." << std::endl;
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