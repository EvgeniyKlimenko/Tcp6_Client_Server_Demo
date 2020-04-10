#include "CommonDefinitions.h"
#include "Client.h"

AsioClient::AsioClient(const char* addr, uint16_t port)
: m_sock(m_ioSvc)
, m_endpoint(boost::asio::ip::address::from_string(addr), port)
{
    m_sock.connect(m_endpoint);
    auto peer = m_sock.remote_endpoint();
    std::cout << "Client " << m_endpoint.address().to_string() << "(" 
        << m_endpoint.port() << ") connected to " 
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
    std::cout << "Finishing ASIO client..." << std::endl;
    m_sock.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    std::cout << "ASIO client finished." << std::endl;
}

#if defined(_WIN64)

 CWinSockClient::CWinSockClient(const char* addr, uint16_t port)
 {
     m_sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
     if (m_sock == INVALID_SOCKET) throw CWindowsException(WSAGetLastError());

    addrinfo hint = {0};
    std::stringstream portHint;
    portHint << port;
    int res = getaddrinfo(addr, portHint.str().c_str(), &hint, &m_addrInfo);
    if(res) throw CWindowsException(res);

    if (connect(m_sock, m_addrInfo->ai_addr, static_cast<int>(m_addrInfo->ai_addrlen)) == SOCKET_ERROR)
        throw CWindowsException(WSAGetLastError());

        std::cout << "Client connected to " << addr << "(" << port << ")." << std::endl;
 }

void CWinSockClient::OnRun()
{
    std::cout << "Client started. Press close button to exit." << std::endl;

    for(;;)
    {
        std::string input;
        std::getline(std::cin, input);

        int res = send(m_sock, input.c_str(), static_cast<int>(input.length()), 0);
        if(res == SOCKET_ERROR)
        {
            std::cerr << "Error writing data: " << CWindowsException::GetErrorDescription(WSAGetLastError()) << std::endl;
        }
        else
        {
            std::cout << "Data " << input << " delivered to the server." << std::endl;
        }

        std::fill(std::begin(m_data), std::end(m_data), 0);
        res = recv(m_sock, &m_data[0], BUF_SIZE, 0);
        if (res == SOCKET_ERROR)
        {
            std::cerr << "Error reading data: " << CWindowsException::GetErrorDescription(WSAGetLastError()) << std::endl;
        }
        else
        {
            std::string dataReceived(&m_data[0], std::min<size_t>(res, input.length()));
            std::cout << "Data received: " << dataReceived << std::endl;
        }
    }
}

void CWinSockClient::OnStop()
{
    std::cout << "Finishing WinSock client..." << std::endl;
    freeaddrinfo(m_addrInfo);
    shutdown(m_sock, SD_BOTH);
    closesocket(m_sock);
    std::cout << "WinSock client finished." << std::endl;
}

#endif // _WIN64
