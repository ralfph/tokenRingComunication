//
// Created by rpych on 08.03.19.
//

#include "TokenRingClient.h"

namespace token {


    std::string TokenRingClient::msgs[7] = {"Ala ma kota", "sockety TCP", "AGH Krakow", "null", "krakowski smok",
                                            "to wiadomosc", "null"};

    TokenRingClient::TokenRingClient(const std::string &clientID, int listeningPort,
                                     const std::string &destinationIPAddress, int destinationPort,
                                     bool hasTokenAtStart, const std::string &protocol) : clientID(clientID),
                                                                                          listeningPort(listeningPort),
                                                                                          destinationIPAddress(
                                                                                                  destinationIPAddress),
                                                                                          destinationPort(
                                                                                                  destinationPort),
                                                                                          hasTokenAtStart(
                                                                                                  hasTokenAtStart),
                                                                                          protocol(protocol) {


        fillSockBindAddrStruct();
        fillSockDestAddrStruct();
        std::strcpy(tokenBuffer, "");
    }

    TokenRingClient::TokenRingClient() {
        fillSockBindAddrStruct();
        fillSockDestAddrStruct();
        std::strcpy(tokenBuffer, "");
    }

    TokenRingClient::~TokenRingClient() {
        if (protocol == "tcp") {
        shutdown(receivingSocket, SHUT_RDWR);
        shutdown(bindingSocket, SHUT_RDWR);
        shutdown(sendingSocket, SHUT_RDWR);
    }
        close(receivingSocket);
        close(bindingSocket);
        close(sendingSocket);
    }

    void TokenRingClient::fillSockBindAddrStruct() {
        socketBindAddress.sin_family = AF_INET;
        socketBindAddress.sin_port = htons((uint16_t) listeningPort);
        inet_pton(AF_INET, destinationIPAddress.c_str(), &socketBindAddress.sin_addr);
    }

    void TokenRingClient::fillSockDestAddrStruct(){
        socketDestAddress.sin_family = AF_INET;
        socketDestAddress.sin_port = htons((uint16_t) destinationPort);
        inet_pton(AF_INET, destinationIPAddress.c_str(), &socketDestAddress.sin_addr);
    }

    void TokenRingClient::receiveTokenFromNeighbourTcp() {
        if (recv(receivingSocket, tokenBuffer, sizeof(tokenBuffer), 0) == -1) {
            std::cout << "Error in recv\n";
            //fprintf(stderr, "recv: %s (%d)\n", strerror(errno), errno);
            return;
        }
    }

    void TokenRingClient::acceptConnectionWithNeighbour() {
        socklen_t addrlen = sizeof(socketSourceAddress);
        receivingSocket = accept(bindingSocket, (sockaddr *) &socketSourceAddress, &addrlen);
        if(receivingSocket > 0){
            isNeighbourAccepted = true;
        }
    }

    void TokenRingClient::bindClientSocketTcp() { //with socket on which receiving msgs
        bindingSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (bindingSocket == -1) {
            std::cout << "Error in bindClientSocket() socket\n";
            return;
        }
        /*int flags = fcntl(bindingSocket, F_GETFL);
        if(fcntl(bindingSocket, F_SETFL, flags | O_NONBLOCK) < 0){
            std::cout << "Error in fcntl() bindClientSocket() socket\n";
            return;
        }*/
        if (bind(bindingSocket, (sockaddr *) &socketBindAddress, sizeof(socketBindAddress)) < 0) {
            std::cout << "Error in bind bindClientSocket() socket\n";
            return;
        }

        if (listen(bindingSocket, 10) < 0) {
            std::cout << "Error in listen bindClientSocket() socket\n";
            return;
        }
    }


    void TokenRingClient::connectClientToNeighbourSocketTcp() {
        sleep(5);
        sendingSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (sendingSocket == -1) {
            std::cout << "Error in connectClientToNeighbourSocket\n";
            return;
        }

        int connResult = connect(sendingSocket, (sockaddr *) &socketDestAddress, sizeof(socketDestAddress));
        if (connResult == -1) {
            std::cout << "Error in connectClientToNeighbourSocket() connect socket\n";
            return;
        }

    }


    void TokenRingClient::passTokenToNeighbourTcp() {
        if (send(sendingSocket, tokenBuffer, strlen(tokenBuffer), 0) <= 0) {
            std::cout << "Error in client send socket\n";
            return;
        }
    }


    void TokenRingClient::sendNormalMsg(std::string& msg){
        std::strcpy(tokenBuffer, "");
        memset(tokenBuffer, 0 ,sizeof(tokenBuffer));
        std::strcpy(tokenBuffer, "OLD");
        std::strcat(tokenBuffer, destinationIPAddress.c_str());
        std::string dPort = std::to_string(destinationPort);
        std:strcat(tokenBuffer, dPort.c_str());
        std::strcat(tokenBuffer, msg.c_str());
        std::cout<<"Msg from sendNormalMsg(): "<<tokenBuffer<<std::endl;
        if(protocol == "tcp")
            passTokenToNeighbourTcp();
        else if(protocol == "udp")
            passTokenToNeighbourUdp();
        std::strcpy(tokenBuffer, "");
    }

    void TokenRingClient::processReceivedMsg() {
        std::string msg(tokenBuffer);
        std::string id = msg.substr(0,3);
        std::string ipAddress = msg.substr(3, 9); //address of new client
        std::string receiverPort_s = msg.substr(12, 5); //listening port of old client
        int receiverPort = atoi(receiverPort_s.c_str());
        std::string msgData_s = msg.substr(17); //listening port of new client
        int msgData = atoi(msgData_s.c_str());

        std::string msgToSend;
        if(id == "NEW" && receiverPort == destinationPort){
            if(receiverPort != msgData) {
                destinationPort = msgData;
                fillSockDestAddrStruct();
                if (!isConnectedToNeighbour) {
                    connectClientToNeighbourSocketTcp();
                }
                //std::cout << "From NEW: " << tokenBuffer << std::endl;
                std::strcpy(tokenBuffer, "");
                msgToSend = "CONNECTED";
                sendNormalMsg(msgToSend);
            }
        }
        else if(id == "NEW" && receiverPort != destinationPort){
            //std::cout<<"Msg from NEW proxy client: "<<tokenBuffer<<std::endl;
            passTokenToNeighbourTcp();
        }

        else if(id == "OLD" && receiverPort == listeningPort){ //hasToken
            msgToSend = msgs[random()%7];
            std::cout<<"Received raw msg: "<<tokenBuffer<<std::endl;
            if(msgToSend != "null")
                sendNormalMsg(msgToSend);
            else {
                //passTokenToNeighbourTcp();
                msgToSend = " ";
                sendNormalMsg(msgToSend);
            }
        }
        else if(id == "OLD"){
            //std::cout<<"Msg from OLD proxy client: "<<tokenBuffer<<std::endl;
            passTokenToNeighbourTcp();
        }
        msgToSend = "";
    }

    void TokenRingClient::runClient() {
        std::cout<<"Client with listening port: "<<listeningPort<<std::endl;
        //receiving part
        bindClientSocketTcp();
        sleep(4);
        if (!TokenRingClient::hasTokenAtStart) {       //both blocking methods
            connectClientToNeighbourSocketTcp();
            sendRequestToJoinSystemTcp();
        }
        //token passing part
        acceptConnectionWithNeighbour();
        while(1){
            if (receivingSocket < 0) {
                if(errno == EWOULDBLOCK) {
                    if(!isNeighbourAccepted)
                        continue;
                }
                else {
                    std::cout << "Error in accept server socket\n";
                    return;
                }
            }
            receiveTokenFromNeighbourTcp();
            sleep(1);
            processReceivedMsg();
        }
    }

    //tokenBuffer msg: NEW, OLD, ADDRESS, PORTNUM, msgData
    void TokenRingClient::sendRequestToJoinSystemTcp(){
        std::strcpy(tokenBuffer, "");
        std::strcpy(tokenBuffer, "NEW");
        std::strcat(tokenBuffer, destinationIPAddress.c_str());
        std::string dPort = std::to_string(destinationPort); //listening port of old client
        std:strcat(tokenBuffer, dPort.c_str());
        std::string lPort = std::to_string(listeningPort); //listening port of new client
        std::strcat(tokenBuffer, lPort.c_str());
        if(protocol == "tcp")
            passTokenToNeighbourTcp();
        else if(protocol == "udp")
            passTokenToNeighbourUdp();
        std::strcpy(tokenBuffer, "");
    }

    //-------------UDP----------------

    void TokenRingClient::bindClientSocketUdp() {
        bindingSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (bindingSocket == -1) {
            std::cout << "Error in bindClientSocket() socket\n";
            return;
        }
        if (bind(bindingSocket, (sockaddr *) &socketBindAddress, sizeof(socketBindAddress)) < 0) {
            std::cout << "Error in bind bindClientSocket() socket\n";
            return;
        }
    }

    void TokenRingClient::connectClientToNeighbourSocketUdp() {
        sendingSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (sendingSocket == -1) {
            std::cout << "Error in connectClientToNeighbourSocket\n";
            return;
        }

        int connResult = connect(sendingSocket, (sockaddr *) &socketDestAddress, sizeof(socketDestAddress));
        if (connResult == -1) {
            std::cout << "Error in connectClientToNeighbourSocket() connect socket\n";
            return;
        }

    }

    void TokenRingClient::receiveTokenFromNeighbourUdp() {
        std::strcpy(tokenBuffer, "");
        socklen_t slen = sizeof(socketSourceAddress);
        if (recvfrom(bindingSocket, (char*)tokenBuffer, sizeof(tokenBuffer), 0, (sockaddr *) &socketSourceAddress, &slen) == -1) {
            std::cout << "Error in recv\n";
            //fprintf(stderr, "recv: %s (%d)\n", strerror(errno), errno);
            return;
        }
    }

    void TokenRingClient::passTokenToNeighbourUdp() {
        socklen_t slen = sizeof(socketDestAddress);
        if (sendto(bindingSocket, (char*)tokenBuffer, sizeof(tokenBuffer), 0,  (sockaddr *) &socketDestAddress, slen) <= 0) {
            std::cout << "Error in client send socket\n";
            return;
        }
    }


    void TokenRingClient::runClientUdp() {
        std::cout<<"Client with listening port: "<<listeningPort<<std::endl;
        bindClientSocketUdp();
        sleep(4);
        if (!TokenRingClient::hasTokenAtStart) {
            connectClientToNeighbourSocketUdp();
            sendRequestToJoinSystemTcp();
        }
        //token passing part
        while(1){
            receiveTokenFromNeighbourUdp();
            sleep(1);
            processReceivedMsgUdp();
        }

    }

    void TokenRingClient::processReceivedMsgUdp() {
        std::string msg(tokenBuffer);
        std::string id = msg.substr(0,3);
        std::string ipAddress = msg.substr(3, 9); //address of new client
        std::string receiverPort_s = msg.substr(12, 5); //listening port of old client
        int receiverPort = atoi(receiverPort_s.c_str());
        std::string msgData_s = msg.substr(17); //listening port of new client
        int msgData = atoi(msgData_s.c_str());

        std::string msgToSend;
        if(id == "NEW" && receiverPort == destinationPort){
            if(receiverPort != msgData) {
                destinationPort = msgData;
                fillSockDestAddrStruct();
                if (!isConnectedToNeighbour) {
                    connectClientToNeighbourSocketUdp();
                }
                //std::cout << "From NEW: " << tokenBuffer << std::endl;
                std::strcpy(tokenBuffer, "");
                msgToSend = "CONNECTED";
                sendNormalMsg(msgToSend);
            }
        }
        else if(id == "NEW" && receiverPort != destinationPort){
            //std::cout<<"Msg from NEW proxy client: "<<tokenBuffer<<std::endl;
            passTokenToNeighbourUdp();
        }

        else if(id == "OLD" && receiverPort == listeningPort){ //hasToken
            msgToSend = msgs[random()%7];
            std::cout<<"Received raw msg: "<<tokenBuffer<<std::endl;
            if(msgToSend != "null")
                sendNormalMsg(msgToSend);
            else {
                //passTokenToNeighbourTcp();
                msgToSend = " ";
                sendNormalMsg(msgToSend);
            }
        }
        else if(id == "OLD"){
            //std::cout<<"Msg from OLD proxy client: "<<tokenBuffer<<std::endl;
            passTokenToNeighbourUdp();
        }
        msgToSend = "";
    }



    void TokenRingClient::chooseClientByProtocol(){
        if(protocol == "tcp"){
            runClient();
        }
        else if(protocol == "udp"){
            runClientUdp();
        }
    }

}

