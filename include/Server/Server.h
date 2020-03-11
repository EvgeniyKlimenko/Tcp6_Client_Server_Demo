#if !defined(__SERVER_H__)
#define __SERVER_H__

class ServerImpl
{
public:
    ServerImpl(short port);

    void Run();
    void Stop();
};

#endif // _SERVER_H__