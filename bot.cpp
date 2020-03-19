#include "bot.hpp"
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <time.h>

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
            int bytes_received = recv(socket_peer, read, 4096, 0);
            if(bytes_received < 1){
                std::cout << "Connection closed by peer\n";
                break;
            }
            std::cout << "Received" << bytes_received << bytes_received << read << std::endl;
        }

        //CHECK PEER_SOCKET TO SEE IF IT HAS ANYTHING TO SEND
        #if defined(_WIN32)
        if(_kbhit()){
        #else
        if(FD_ISSET(0, &reads)){
        #endif

            char read[4096];
            if(!fgets(read, 4096, stdin)) break;
            std::cout << "Sending:" << read;
            int bytes_sent = send(socket_peer, read, strlen(read), 0);
            std::cout << "Sent " << bytes_sent << " bytes\n";
        }
    }
}

bot::bot(){
    if(getaddrinfo(serv_addr,port, &hints, &peer_address)){
        std::cout << stderr << "getaddressinfo() failed\n" << GETSOCKETERRNO() << std::endl;
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
}