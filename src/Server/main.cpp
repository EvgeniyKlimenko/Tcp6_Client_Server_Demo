#include "AppLogic.h"
#include "Server.h"

namespace opt = boost::program_options;

static const short DEFAULT_PORT = 15000;

int main(int argc, char* argv[])
{
    opt::options_description desc("TCPv6 server options");
    desc.add_options()
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

    short port = (varMap.count("port")) ? varMap["port"].as<short>() : DEFAULT_PORT;
    RUN_APP(AsioServer, port);

    return 0;
}