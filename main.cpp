
#include <tuple>
#include "TokenRingClient.h"
using namespace token;


std::tuple<std::string, int> processDestinationAddress(std::string &destinationAddress) {
        long pos = destinationAddress.find(":");
        std::string ipAddress = destinationAddress.substr(0, pos);
        std::string s_port = destinationAddress.substr(pos + 1);
        int port = atoi(s_port.c_str());
        return std::make_tuple(ipAddress, port);
    }

    int main(int argc, char** argv) {

        if(argc !=6 ) {
            std::cout<<"Wrong number of parameters"<<std::endl;
            exit(1);
        }
        std::string clientID = argv[1];
        std::string lpStr = argv[2];
        int listeningPort = atoi(lpStr.c_str());
        std::string ipAddressTmp = argv[3];
        auto address = processDestinationAddress(ipAddressTmp);
        uint16_t destPort = std::get<1>(address);
        std::string ipAddress = std::get<0>(address);

        bool hasTokenAtStart = false;
        if(strcmp(argv[4], "true") == 0){
            hasTokenAtStart = true;
        }
        else if(strcmp(argv[4], "false") == 0) {
            hasTokenAtStart = false;
        }
        std::string protocol = argv[5];
        TokenRingClient client(clientID, listeningPort, ipAddress, destPort, hasTokenAtStart, protocol);

        client.chooseClientByProtocol();

        return 0;
    }

