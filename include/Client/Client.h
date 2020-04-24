#if !defined(__CLIENT_H__)
#define __CLIENT_H__

#include "AppLogic.h"
#include "System/WinSockIniter.h"

class AsioClient final : public AppLogic<AsioClient, false>
{
public:
    AsioClient(const char* addr, uint16_t port);
    ~AsioClient() { Stop(); }

    void OnRun();
    void OnStop();

private:
    static const size_t BUF_SIZE = 1024;
    boost::asio::io_service m_ioSvc;
    boost::asio::ip::tcp::socket m_sock;
    boost::asio::ip::tcp::endpoint m_endpoint;
    boost::array<char, BUF_SIZE> m_data;
};

template <typename T> struct DescrDeleter
{
    DescrDeleter() = default;
    void operator()(T) const;
};

template <typename E> class ErrorCheck
{
public:
    bool Failed();
};

#if defined (_WIN64)
template <> class ErrorCheck<SOCKET>
{
    SOCKET m_s;
public:
    ErrorCheck(SOCKET& s) : m_s(s) {}
    
    bool Failed() { return m_s == INVALID_SOCKET; }
};

template <> class ErrorCheck<int>
{
    int m_e;
public:
    ErrorCheck(int& e) : m_e(e) {}
    
    bool Failed() { return m_e == SOCKET_ERROR; }
};

#elif defined (__linux__)

template <> class ErrorCheck<int>
{
    int m_e;
public:
    ErrorCheck(int& e) : m_e(e) {}
    
    bool Failed() { return m_e < 0; }
};

#endif // _WIN64

template
<
    typename Descriptor,
    template <typename> typename Deleter,
    typename Exception,
    typename SocketSubsystemIniter,
    int ShutdownFlags
>
class SystemClient : public AppLogic
<
    SystemClient
    <
        Descriptor,
        Deleter,
        Exception,
        SocketSubsystemIniter,
        ShutdownFlags
    >, false
>
{
    static const size_t BUF_SIZE = 1024;
    SocketSubsystemIniter m_subsysIniter;
    Deleter<Descriptor> m_deleter;
    Descriptor m_sock;
    addrinfo* m_addrInfo;
    boost::array<char, BUF_SIZE> m_data;

public:
    SystemClient(const char* addr, uint16_t port)
    : m_addrInfo(nullptr)
    {
        m_sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
        if (ErrorCheck<Descriptor>(m_sock).Failed()) throw Exception();

        addrinfo hint;
        memset(&hint, 0, sizeof(hint));

        std::stringstream portHint;
        portHint << port;
        
        int res = getaddrinfo(addr, portHint.str().c_str(), &hint, &m_addrInfo);
        if(ErrorCheck<int>(res).Failed()) throw Exception(res);

        res = connect(m_sock, m_addrInfo->ai_addr, static_cast<int>(m_addrInfo->ai_addrlen));
        if (ErrorCheck<int>(res).Failed()) throw Exception();

        std::cout << "Client connected to " << addr << "(" << port << ")." << std::endl;
    }

    ~SystemClient() { this->Stop(); }

    void OnRun()
    {
        std::cout << "Client started. Press close button to exit." << std::endl;

        for(;;)
        {
            std::string input;
            std::getline(std::cin, input);

            int res = send(m_sock, input.c_str(), static_cast<int>(input.length()), 0);
            if(ErrorCheck<int>(res).Failed())
            {
                std::cerr << "Error writing data: " << Exception::GetErrorDescription() << std::endl;
                break;
            }
            else
            {
                std::cout << "Data " << input << " delivered to the server." << std::endl;
            }

            std::fill(std::begin(m_data), std::end(m_data), 0);
            res = recv(m_sock, &m_data[0], BUF_SIZE, 0);
            if (ErrorCheck<int>(res).Failed())
            {
                std::cerr << "Error reading data: " << Exception::GetErrorDescription() << std::endl;
                break;
            }
            else
            {
                std::string dataReceived(&m_data[0], std::min<size_t>(res, input.length()));
                std::cout << "Data received: " << dataReceived << std::endl;
            }
        }
    }

    void OnStop()
    {
        std::cout << "Finishing system API based client..." << std::endl;
        freeaddrinfo(m_addrInfo);
        shutdown(m_sock, ShutdownFlags);
        m_deleter(m_sock);
        std::cout << "System API based client finished." << std::endl;
    }
};


#if defined (_WIN64)

template <> struct DescrDeleter<SOCKET>
{
    void operator()(SOCKET s) { closesocket(s); }
};

using CWinSockClient = SystemClient
    <
        SOCKET, DescrDeleter, CWinSockExceptionDecorator, CWinSockIniter, SD_BOTH
    >; 

#elif defined (__linux__)

template <> struct DescrDeleter<int>
{
    void operator()(int s) { close(s); }
};

using LinuxClient = SystemClient
    <
        int, DescrDeleter, SystemExceptionDecorator, SubsysIniterNullObj, SHUT_RDWR
    >; 

#endif

#if defined(USE_NATIVE)

#if defined(_WIN64)

using CurrentClient = CWinSockClient;

#elif defined(__linux__)

using CurrentClient = LinuxClient;

#endif

#else

using CurrentClient = AsioClient;

#endif // USE_NATIVE

#endif // __CLIENT_H__