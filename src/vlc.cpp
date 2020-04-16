#include "../include/vlc.hpp"

vlc::vlc(){
    inst = libvlc_new (0, NULL);
    state= -1;
    cont = 0;
}

bool vlc::checkDownload(std::string url){
    if(url.find("list=") == std::string::npos && url.find("radio") == std::string::npos && (url.find("youtube.com") != std::string::npos || url.find("youtu.be") != std::string::npos)){
        return true;
    }
    else{
        std::cout << "Check Download failed\n";
        return false;
    }
    return false;
}

bool vlc::addToRequestList(std::string url){
    if(checkDownload(url)){
        std::thread dwnld(vlcDownload,url);
        dwnld.detach();
        requestList.push_back(url);
        return true;
    }
    return false;
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

    for(int i=0;!(exists(std::string("./music/") + url.substr(url.find_last_of("/")+1))) && i<10;i++){
        usleep(1000000);
    }
    m = libvlc_media_new_path (inst, (std::string("./music/") + url.substr(url.find_last_of("/")+1)).c_str());
    mp = libvlc_media_player_new_from_media (m);
    libvlc_media_release (m);
    libvlc_media_player_play (mp);
    state = libvlc_media_player_get_state(mp);
    return;
}

void vlc::vlcSkip(){
    libvlc_media_player_stop(mp);
    return;
}

void vlc::vlcChangeVolume(int volume){
    libvlc_audio_set_volume(mp,volume%101);
    return;
}

bool vlc::exists(std::string name){
    std::ifstream f(name.c_str());
    return f.good();
}

void vlc::vlcDownload(std::string url){
    std::string command = "youtube-dl -f 'bestaudio[filesize<15M]' -o ./music/" + url.substr(url.find_last_of("/")+1) + " " + url + " --no-playlist --geo-bypass";
    system(command.c_str());
    system("youtube-dl --rm-cache-dir");
    return;
}