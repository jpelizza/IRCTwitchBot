#ifndef VLC
#define VLC

#if defined(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

#include <vlc/vlc.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <regex>
#include <tuple>
#include <list>
#include <vector>
#include <random>
#include <algorithm>
#include <iterator>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")

#else

#include <unistd.h>
#include <vlc/vlc.h>
#include <sys/socket.h>
#include <iostream>
#include <thread>
#include <fstream>
#include <iostream>
#include <regex>
#include <tuple>
#include <list>
#include <vector>
#include <random>
#include <algorithm>
#include <iterator>
#include <iostream>

#endif




class vlc{
    //VLC ESSENTIALS
    public:
    libvlc_instance_t * inst;
    libvlc_media_player_t *mp;
    libvlc_media_t *m;

    //USEFUL
    public:
    std::vector<std::string> standByPlaylist;
    std::list<std::tuple<std::string,std::string>> requestList;
    int state;
    int cont;
    bool ableToPlay;
    std::random_device rd;

        //REGEX
    std::cmatch c;
    
    //FILES
    std::ifstream myfile;
    std::string title;

    public:
    vlc();
    static void vlcDownload(std::string);
    static void getTitle(std::string);
    bool checkDownload(std::string);
    int addToRequestList(std::string);
    void vlcPlay(std::string);
    std::string checkOnPlayer();
    bool exists(std::string);
    void vlcSkip();
    void vlcChangeVolume(int);
    void getStandByPlaylist();
    std::string searchByName(std::string);
    void getTime();


    
};

#endif // VLC