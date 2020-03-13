#if !defined(__CLIENT_H__)
#define __CLIENT_H__

#include "AppLogic.h"

class AsioClient final : public AppLogic<AsioClient>
{
public:
    AsioClient(const std::string& addr, uint16_t port);
    ~AsioClient() { Stop(); }

    void OnRun();
    void OnStop();

private:
    void ReaderCallback();

private:
    volatile bool m_stop;
    boost::asio::io_service m_ioSvc;
    boost::asio::ip::tcp::socket m_sock;
    boost::asio::ip::tcp::endpoint m_endpoint;
    boost::thread m_reader;
};

#endif // __CLIENT_H__