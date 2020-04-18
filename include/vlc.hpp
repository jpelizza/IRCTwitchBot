#ifndef VLC
#define VLC


#include <unistd.h>
#include <vlc/vlc.h>
#include <sys/socket.h>
#include <iostream>
#include <list>
#include <thread>
#include <fstream>
#include <iostream>
#include <regex>
#include <tuple>


class vlc{
    //VLC ESSENTIALS
    public:
    libvlc_instance_t * inst;
    libvlc_media_player_t *mp;
    libvlc_media_t *m;

    //USEFUL
    public:
    std::list<std::tuple<std::string,std::string>> requestList;
    int state;
    int cont;
    bool ableToPlay;

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


    
};

#endif // VLC