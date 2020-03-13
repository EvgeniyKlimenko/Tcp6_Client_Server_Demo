#include "CommonDefinitions.h"
#include "Client.h"

AsioClient::AsioClient(const std::string& addr, uint16_t port)
: m_sock(m_ioSvc)
, m_stop(false)
, m_endpoint(boost::asio::ip::address::from_string(addr), port)
{
    m_sock.connect(m_endpoint);
    auto peer = m_sock.remote_endpoint();
    std::cout << "Client " << m_endpoint.address().to_string() << "(" 
        << m_endpoint.port() << ") connected to the server " 
        << peer.address().to_string() 
        << "(" << peer.port() << ")." << std::endl;
    m_reader = boost::thread(boost::bind(&AsioClient::ReaderCallback, this));
}

void AsioClient::OnRun()
{
    std::cout << _T("Client started. Press close button to exit.") << std::endl;

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
            std::cout << "Data delivered to the server." << std::endl;
        }
    }
}

void AsioClient::OnStop()
{
    std::cout << "Finishing client..." << std::endl;
    m_stop = true;
    m_reader.join();
    std::cout << "Client finished." << std::endl;
}

void AsioClient::ReaderCallback()
{
    try
    {
        for(;;)
        {
            if (m_stop) break;

            boost::system::error_code err;
            boost::asio::streambuf data;
            boost::asio::read(m_sock, data, boost::asio::transfer_all(), err);
            if (err && err != boost::asio::error::eof)
            {
                std::cerr << "Error reading data: " << err.message() << std::endl;
            }
            else
            {
                std::string dataReceived;
                std::istream istm(&data);
                istm >> dataReceived;

                std::cout << "Data received: " << dataReceived << std::endl;
            }
        }
    }
    catch(...)
    {
        m_exceptioning.Append(boost::current_exception());
    }
}
