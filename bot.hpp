/*sock_init.cpp*/

#ifndef BOT
#define BOT

#if defined(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <string.h>
#include <stdio.h>

#endif


#if defined(_WIN32)
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#define CLOSESOCKET(s) closesocket(s)
#define GETSOCKETERRNO() (WSAGetLastError())

#else
#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)
#endif

struct msg{
    std::string text;
    std::string user;
};

/*botClass*/
class bot{
    public: //variables

        
    //BOT'S INFO
    const char* oauth = "oauth:###\n";
    const char* nick = "nick\n";
    const char* channel = "#channel";
    
    //NETOWORK VARIABLES
    SOCKET socket_peer;
    struct addrinfo hints;
    struct addrinfo *peer_address;
    struct timeval timeout;
    fd_set reads;
    
    //TIME VARIABLES
    time_t pingTimer;

    //TWITCH ADDRS & PORT
    const char* serv_addr = "irc.chat.twitch.tv";
    const char* port = "6667";

    //DEV INFO
    bool devMode = true;
    bool chatMode =true;

    //USEFULNESS
    double minutes;
    std::string privmsg = "PRIVMSG ";
    int bytes_recv = 0;
    struct msg latestMsg;

    public: //functions
    //CONSTRUCTOR
    bot();
    //MAINTENANCE
    void loop();
    void pong();
    void ping();
    void login();
    //MESSAGES
    void msgCheck(char *recv);
    struct msg msgManager(char *recv);
    //COMANDS
    void sendprivmsg(std::string);
    void Cdice(struct msg);
};

#endif // BOT