#include "CommonDefinitions.h"
#include "Client.h"

AsioClient::AsioClient(const char* addr, uint16_t port)
: m_sock(m_ioSvc)
, m_endpoint(boost::asio::ip::address::from_string(addr), port)
{
    m_sock.connect(m_endpoint);
    auto peer = m_sock.remote_endpoint();
    std::cout << "Client " << m_endpoint.address().to_string() << "(" 
        << m_endpoint.port() << ") connected to the server " 
        << peer.address().to_string() 
        << "(" << peer.port() << ")." << std::endl;
}

void AsioClient::OnRun()
{
    std::cout << "Client started. Press close button to exit." << std::endl;

    for(;;)
    {
        std::string input;
        std::getline(std::cin, input);

        boost::system::error_code err;
        boost::asio::write(m_sock, boost::asio::buffer(input), err);
        if(err)
        {
            std::cerr << "Error writing data: " << err.message() << std::endl;
        }
        else
        {
            std::cout << "Data " << input << " delivered to the server." << std::endl;
        }

        std::fill(std::begin(m_data), std::end(m_data), 0);
        size_t bytesRead = boost::asio::read(m_sock, boost::asio::buffer(m_data), boost::asio::transfer_all(), err);
        if (err && err != boost::asio::error::eof)
        {
            std::cerr << "Error reading data: " << err.message() << std::endl;
        }
        else
        {
            std::string dataReceived(&m_data[0], std::min<size_t>(bytesRead, input.length()));
            std::cout << "Data received: " << dataReceived << std::endl;
        }
    }
}

void AsioClient::OnStop()
{
    std::cout << "Finishing client..." << std::endl;
    m_sock.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    std::cout << "Client finished." << std::endl;
}
