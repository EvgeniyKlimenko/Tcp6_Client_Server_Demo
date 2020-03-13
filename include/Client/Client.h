#if !defined(__CLIENT_H__)
#define __CLIENT_H__

#include "AppLogic.h"

class AsioClient final : public AppLogic<AsioClient>
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
    char m_data[BUF_SIZE];
};

#endif // __CLIENT_H__