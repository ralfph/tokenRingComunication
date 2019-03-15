//
// Created by rpych on 08.03.19.
//
#include <iostream>
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <zconf.h>
#include <sys/fcntl.h>
#include <errno.h>


#ifndef TOKENRINGCOMMUNICATION_TOKENRINGTCPCLIENT_H
#define TOKENRINGCOMMUNICATION_TOKENRINGTCPCLIENT_H

namespace token {

    class TokenRingClient {
    public:
        static std::string msgs[7];
        bool hasTokenNow = false;
        char tokenBuffer[1024];
        char spareBuffer[1024];

        TokenRingClient(const std::string &clientID, int listeningPort, const std::string &destinationIPAddress,
                        int destinationPort,
                        bool hasTokenAtStart, const std::string &protocol);

        TokenRingClient();

        ~TokenRingClient();

        void fillSockBindAddrStruct();

        void runClient();

        void receiveTokenFromNeighbourTcp();

        void bindClientSocketTcp();

        void connectClientToNeighbourSocketTcp();

        void acceptConnectionWithNeighbour();

        void passTokenToNeighbourTcp();

        void sendRequestToJoinSystemTcp();

        void processReceivedMsg();

        void sendNormalMsg(std::string& msg);

        void fillSockDestAddrStruct();

        //-----UDP-----
        void bindClientSocketUdp();

        void connectClientToNeighbourSocketUdp();

        void receiveTokenFromNeighbourUdp();

        void passTokenToNeighbourUdp();

        void runClientUdp();

        void processReceivedMsgUdp();

        void chooseClientByProtocol();

    private:
        std::string clientID;
        int listeningPort;
        std::string destinationIPAddress;
        int destinationPort;
        bool hasTokenAtStart;
        std::string protocol;

        sockaddr_in socketBindAddress;
        sockaddr_in socketDestAddress;
        sockaddr_in socketSourceAddress;
        int bindingSocket;
        int sendingSocket;
        int receivingSocket;
        bool isNeighbourAccepted = false;
        bool isConnectedToNeighbour = false;
    };


}


#endif //TOKENRINGCOMMUNICATION_TOKENRINGTCPCLIENT_H
