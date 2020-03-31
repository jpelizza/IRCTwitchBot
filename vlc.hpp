#include <unistd.h>
#include <vlc/vlc.h>
#include <iostream>
#include <list>
#include <thread>
#include <fstream>


class vlc{
    libvlc_instance_t * inst;
    libvlc_media_player_t *mp;
    libvlc_media_t *m;

    std::list<std::string> requestList;
    int state;
    int cont;
    public:
    vlc();
    void addToRequestList(std::string);
    void vlcPlay(std::string);
    static void vlcDownload(std::string);
    void checkOnPlayer();
    bool exists(std::string);

};

