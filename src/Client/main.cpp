#include "AppLogic.h"
#include "Client.h"

namespace opt = boost::program_options;

static const char DEFAULT_HOST[] = "::1";
static const short DEFAULT_PORT = 15000;

int main(int argc, char* argv[])
{
    opt::options_description desc("TCPv6 client options");
    desc.add_options()
    ("address,a", opt::value<std::string>()->default_value(DEFAULT_HOST))
    ("port,p", opt::value<short>()->default_value(DEFAULT_PORT))
    ("help,h", "see this help text");

    opt::variables_map varMap;
    opt::store(opt::parse_command_line(argc, argv, desc), varMap);
    opt::notify(varMap);

    if(varMap.count("help"))
    {
        std::cout << desc << std::endl;
        return 1;
    }

    const char* host = (varMap.count("host")) ? varMap["host"].as<std::string>().c_str() : DEFAULT_HOST;
    short port = (varMap.count("port")) ? varMap["port"].as<short>() : DEFAULT_PORT;
    RUN_APP(AsioClient, host, port);

    return 0;
}