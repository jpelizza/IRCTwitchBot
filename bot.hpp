/*sock_init.cpp*/



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


/*botClass*/
class bot{
    public: //variables
    SOCKET socket_peer;
    struct addrinfo hints;
    struct addrinfo *peer_address;
    fd_set reads;
    struct timeval timeout;
    time_t pingTimer;
    //Timer pingTimer = Timer();
    char address_buffer[100], service_buffer[100];
    const char* serv_addr = "irc.chat.twitch.tv";
    const char* port = "6667";

    const char* aoth = "oauth:###";
    const char* nick = "channelName";

    public: //functions
    bot();
    void loop();
    int ping();
    void login();
};