#include "bot.hpp"
#include <stdio.h>
#include <string>
#include <string.h>
#include <iostream>
#include <time.h>

bot::bot(){
    sleep(1);
    for(int i=0;(getaddrinfo(serv_addr,port, &hints, &peer_address)) && i<5; i++){
        std::cout << stderr << "getaddressinfo() failed\n" << GETSOCKETERRNO() << std::endl;
        std::cout << "Trying again. Please wait.\n";
        sleep(1);
    }

    memset(&hints,0,sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    
    getnameinfo(peer_address->ai_addr,
    peer_address->ai_addrlen,
    address_buffer, sizeof(address_buffer),
    service_buffer, sizeof(service_buffer),
    NI_NUMERICHOST);


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
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;

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
    char data[4096]  = "PONG\n";
    int bytes_sent = send(socket_peer,data,strlen(data),0);
    if(bytes_sent > 1 && devMode) std::cout << "PONG sent\n";
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
        if(select(socket_peer+1, &reads, 0, 0, &timeout) <0){
            std::cout << stderr << "select() failed\n" << GETSOCKETERRNO();
        }
        //CHECK PEER SOCKET TO SEE IF ANYTHING WAS RECEIVED
        if(FD_ISSET(socket_peer, &reads)){
            char read[4096];
            bytes_recv = recv(socket_peer, read, 4096, 0);
            std::string aux = read;
            //TAKES CARE OF MESSAGE
            if (strcmp(read,"PING")==0) pong();
            else if(aux.find(channel) != std::string::npos){
                if(devMode) std::cout << "PRIVMSG" << std::endl;
                msgCheck(read);
            }
            //CHECK IF CONNECTION CLOSED
            if(bytes_recv < 1){
                std::cout << "Connection closed by peer\n";
                break;
            }
            //CLEAN CHAR ARRAY
            memset(&read, 0, sizeof(read));
            //PRINT
            if(devMode){
                std::cout << "Received" << bytes_recv << std::endl << read << std::endl;
            }
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
}

void bot::msgCheck(char *msgRecv){
    std::cout << "MsgCheck()\n";
    struct msg latestMsg = msgManager(msgRecv);
    if(latestMsg.text[0] == '!'){
        if(latestMsg.text.compare("!dice")){
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
    struct msg latestMsg;
    std::string aux;
    aux = msgRecv;
    latestMsg.user = aux.substr(1,aux.find('!')-1);
    latestMsg.text = aux.substr(aux.find("PRIVMSG #")+19);
    if(devMode)std::cout << latestMsg.text << std::endl;
    return latestMsg;

}

void bot::Cdice(struct msg latestMsg){
    std::cout << "Cdice()" << std::endl;
    std::string aux = "PRIVMSG " + std::string(channel) + " :@" + latestMsg.user + " rolled " +
                        std::to_string((rand()%20)+1) + "\n";
    std::cout << aux.c_str() << std::endl;
    std::cout << strlen(aux.c_str()) << " " << aux.length() << std::endl;
    int bytes_sent = send(socket_peer,aux.c_str(),strlen(aux.c_str()),0);
    std::cout << bytes_sent;
    return;
}

