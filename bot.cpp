#include "bot.hpp"


bot::bot(){
    if(getaddrinfo(serv_addr,port, &hints, &peer_address)){
        std::cout << stderr << "getaddressinfo() failed\n" << GETSOCKETERRNO() << std::endl;
        std::cout << "Trying again. Please wait.\n";
    }

    memset(&hints,0,sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    //INITIALIZING SOCKET
    socket_peer = socket(peer_address->ai_family,
    peer_address->ai_socktype, peer_address->ai_protocol);
    if(!ISVALIDSOCKET(socket_peer)){
        std::cout << "socket() failed" << GETSOCKETERRNO() << std::endl;
    }

    //CONNECTION
    if(connect(socket_peer, peer_address->ai_addr, peer_address->ai_addrlen)){
        std::cout << stderr << "connect() failed " << GETSOCKETERRNO() << std::endl;
    }

    //SET TIMEOUT ON CONNECTION
    timeout.tv_sec = 360;
    timeout.tv_usec = 100000;

    time(&pingTimer);
    //Initilize Rand
    srand (time(NULL));
    login();
}

void bot::login(){
    char data[4096]  = "PASS ";
    char data2[4096] = "NICK ";
    char data3[4096] = "JOIN ";
    strcat(data,oauth);
    strcat(data2,nick);
    strcat(data3,channel);
    strcat(data3,"\n");
    send(socket_peer,data,strlen(data),0);
    send(socket_peer,data2,strlen(data2),0);
    send(socket_peer,data3,strlen(data3),0);
}

void bot::pong(){
    char data[4096]  = "PONG :tmi.twitch.tv\n";
    int bytes_sent = send(socket_peer,data,strlen(data),0);
    if(bytes_sent > 1 && devMode) std::cout << "PONG sent\n";
}
//NO CURRENT IMPLEMENTATION
void bot::ping(){
    char data[4096]  = "PING :tmi.twitch.tv\n";
    int bytes_sent = send(socket_peer,data,strlen(data),0);
    if(bytes_sent > 1 && devMode) std::cout << "PING sent\n";
}

void bot::loop(){
    while(true){
        //SETUP READS WITH SOCKET
        FD_ZERO(&reads);
        FD_SET(socket_peer, &reads);
        //WINDOWS
        #if !defined(_WIN32)
            FD_SET(0,&reads);
        #endif
        //CHECK IF SELECT WAS OK
        if(select(socket_peer+1, &reads, 0, 0, &timeout) < 0){
            std::cout << stderr << "select() failed\n" << GETSOCKETERRNO();
        }
        //CHECK PEER SOCKET TO SEE IF ANYTHING WAS RECEIVED
        if(FD_ISSET(socket_peer, &reads)){
            char read[4096];
            bytes_recv = recv(socket_peer, read, 4096, 0);
            //TAKES CARE OF MESSAGE
            if (std::string(read).substr(0,4).compare("PING")==0) pong();
            else if(std::string(read).find(channel) != std::string::npos){
                msgCheck(read);
            }
            //CHECK IF CONNECTION CLOSED
            if(bytes_recv < 1){
                std::cout << "Connection closed by peer\n";
                break;
            }
            //PRINT
            if(devMode){
                std::cout << "Received " << bytes_recv << " bytes, message is: " << std::endl << read << std::endl;
            }
            //CLEAN CHAR ARRAY
            memset(&read, 0, sizeof(read));
        }

        //CHECK PEER_SOCKET TO SEE IF IT HAS ANYTHING TO SEND
        #if defined(_WIN32)
        if(_kbhit()){
        #else
        if(FD_ISSET(0, &reads)){
        #endif
            char read[4096];
            if(!fgets(read, 4096, stdin)) break;
            int bytes_sent = send(socket_peer, read, strlen(read), 0);
            if(devMode){
                std::cout << "Sent " << bytes_sent << " bytes\n";
            }
        }
    }

    // std::cout << (difftime(pingTimer,time(NULL))) << std::endl;
    // if((difftime(pingTimer,time(NULL)))/60 > 4){
    //     ping();
    // }

}

void bot::msgCheck(char *msgRecv){
    struct msg latestMsg = msgManager(msgRecv);

    if(latestMsg.text[0] == '!'){
        if(latestMsg.text.compare("!dice")==0){
            Cdice(latestMsg);
        }
    }
    else{
        std::cout << "Não entendi mensagem\n";
    }
    return;
}

struct msg bot::msgManager(char *msgRecv){
    if(devMode) std::cout << "msgManager()\n";

    latestMsg.user = std::string(msgRecv).substr(1,std::string(msgRecv).find('!')-1);
    latestMsg.text = std::string(msgRecv).substr(std::string(msgRecv).find(" :")+2);
    if(chatMode) std::cout << latestMsg.user <<" : " << latestMsg.text << std::endl;
    //string fix
    latestMsg.text.pop_back();latestMsg.text.pop_back();
    
    return latestMsg;

}

void bot::Cdice(struct msg latestMsg){
    std::string msg = privmsg + std::string(channel) + " :@" + latestMsg.user + " rolled " +
                        std::to_string((rand()%20)+1) + "\n";
    sendprivmsg(msg);
}

void bot::sendprivmsg(std::string text){
    int bytes_sent = send(socket_peer,text.c_str(),strlen(text.c_str()),0);
    if(bytes_sent<1){
        std::cout << "ERROR TRYING TO SEND MESSAGE\n";
    }
}
