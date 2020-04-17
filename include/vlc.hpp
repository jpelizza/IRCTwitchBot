#ifndef VLC
#define VLC


#include <unistd.h>
#include <vlc/vlc.h>
#include <iostream>
#include <list>
#include <thread>
#include <fstream>
#include <iostream>
#include <regex>


class vlc{
    //VLC ESSENTIALS
    public:
    libvlc_instance_t * inst;
    libvlc_media_player_t *mp;
    libvlc_media_t *m;

    //USEFUL
    public:
    std::list<std::string> requestList;
    int state;
    int cont;
    bool ableToPlay;

    public:
    vlc();
    bool checkDownload(std::string);
    bool addToRequestList(std::string);
    void vlcPlay(std::string);
    static void vlcDownload(std::string);
    void checkOnPlayer();
    bool exists(std::string);
    void vlcSkip();
    void vlcChangeVolume(int);
};

#endif // VLC