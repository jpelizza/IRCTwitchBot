#include "../include/vlc.hpp"
vlc::vlc(){
    inst = libvlc_new (0, NULL);
    state= -1;
    cont = 0;
}

void vlc::addToRequestList(std::string request){
    requestList.push_back(request);
    return;
}

void vlc::checkOnPlayer(){
    
    if(state!=-1)state = libvlc_media_player_get_state(mp);

    if(state == 0 || state == 1 || state == 3)return;
    else if( requestList.size() > 0 ) {
        vlcPlay(requestList.back());
        requestList.pop_back();
    }

    return;
}

void vlc::vlcPlay(std::string url){
    std::thread dwnld(vlcDownload,url);
    dwnld.detach();
    while(!exists(std::string("./music/") + url.substr(33))){
        usleep(1000);
    }
    m = libvlc_media_new_path (inst, (std::string("./music/") + url.substr(33)).c_str());
    mp = libvlc_media_player_new_from_media (m);
    libvlc_media_release (m);
    libvlc_media_player_play (mp);
    state = libvlc_media_player_get_state(mp);
    return;
}

bool vlc::exists(std::string name){
    std::ifstream f(name.c_str());
    return f.good();
}

void vlc::vlcDownload(std::string url){
    std::string command = "youtube-dl -o ./music/" + url.substr(33) + " " + url + " --format bestaudio --no-playlist";
    system(command.c_str());
    return;
}