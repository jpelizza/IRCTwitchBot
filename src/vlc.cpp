#include "../include/vlc.hpp"

vlc::vlc(){
    inst = libvlc_new (0, NULL);
    state= -1;
    cont = 0;
}
bool vlc::addToRequestList(std::string url){
    std::cmatch c;
    regex_match(url.c_str(),c,std::regex("(?:(.*v=([^&]*).*))|(?:.*be/(.*))"));
    std::cmatch ccopy(c);

    for(unsigned i=0; i<ccopy.size(); i++){
        std::cout << ccopy[i].length() << std::endl;
        if(ccopy[i].length()==11) url = ccopy[i];
    }

    std::cout << url.size() << std::endl;
    if(url.size()!=11) return false;

    std::thread dwnld(vlcDownload,url);
    dwnld.detach();
    requestList.push_back(url);
    return true;
}
void vlc::checkOnPlayer(int sock_peer, std::string privmsg){
    
    if(state!=-1)state = libvlc_media_player_get_state(mp);

    if(state == 0 || state == 1 || state == 3)return;
    
    else if( requestList.size() > 0 ) {
        vlcPlay(requestList.back());
        getNowPlaying(sock_peer, privmsg , requestList.back());
        requestList.pop_back();
    }

    return;
}
void vlc::getNowPlaying(int sock_peer,std::string privmsg, std::string url){
    system(std::string("youtube-dl "+ url +" --get-title > title.txt").c_str());
    std::ifstream myfile;
    std::string fileName = "title.txt";
    std::string output;
    myfile.open(fileName, std::ios::out | std::ios::app | std::ios::binary);
    getline(myfile,output);
    output = privmsg + "Now playing: " + output + "\n";

    send(sock_peer,output.c_str(),output.size(),0);
    return;
}
void vlc::vlcPlay(std::string url){

    for(int i=0;!(exists(std::string("./music/") + url) && i<10);i++){
        usleep(1000000);
    }
    m = libvlc_media_new_path (inst, (std::string("./music/") + url).c_str());
    mp = libvlc_media_player_new_from_media (m);
    libvlc_media_release (m);
    libvlc_media_player_play (mp);
    state = libvlc_media_player_get_state(mp);
    return;
}
void vlc::vlcSkip(){
    if(state==-1){return;}
    libvlc_media_player_stop(mp);
    return;
}
void vlc::vlcChangeVolume(int volume){
    if(state==-1){return;}
    libvlc_audio_set_volume(mp,volume%101);
    return;
}
bool vlc::exists(std::string name){
    std::ifstream f(name.c_str());
    return f.good();
}
void vlc::vlcDownload(std::string url){
    std::string command = "youtube-dl -f 'bestaudio[filesize<15M]' -o ./music/" + url + " " + url + " --no-playlist --geo-bypass";
    system(command.c_str());
    system("youtube-dl --rm-cache-dir");
    return;
}