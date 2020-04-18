#include "../include/vlc.hpp"

vlc::vlc(){
    inst = libvlc_new (0, NULL);
    state= -1;
    cont = 0;
}
int vlc::addToRequestList(std::string url){
    
    regex_match(url.c_str(),c,std::regex("(?:(?:.*v=([^&]*).*))|(?:.*be/(.*))"));
    std::cmatch ccopy(c);

    for(unsigned i=0; i<ccopy.size(); i++){
        std::cout << ccopy[i].length() << std::endl;
        if(ccopy[i].length()==11) url = ccopy[i];
    }

    std::cout << url.size() << std::endl;
    if(url.size()!=11) return 1;

    for(auto i=requestList.begin();i!=requestList.end();i++){
        if(!std::get<0>(*i).compare(url.c_str())){
            return 2;
        }
    }
    std::thread getTitleThread(getTitle,url);
    std::thread dwnld(vlcDownload,url);
    dwnld.detach();
    getTitleThread.join();
    myfile.open("title.txt", std::ios::out | std::ios::app | std::ios::binary);
    getline(myfile,title);
    std::cout << title << std::endl;
    requestList.push_back(std::make_tuple(url,title));
    return -1;
}

void vlc::getTitle(std::string url){
    system(std::string("youtube-dl " + url + " --get-title > title.txt").c_str());
    return;
}

std::string vlc::checkOnPlayer(){
    
    if(state!=-1)state = libvlc_media_player_get_state(mp);

    if(state == 0 || state == 1 || state == 3)return "";
    
    else if( requestList.size() > 0 ) {
        vlcPlay(std::get<0>(requestList.front()));
        title = std::get<1>(requestList.front());
        requestList.pop_front();
        return title;
    }
    return "";
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
    std::ifstream myfile(name.c_str());
    return myfile.good();
}
void vlc::vlcDownload(std::string url){
    std::string command = "youtube-dl -f 'bestaudio[filesize<15M]' -o ./music/" + url + " " + url + " --no-playlist --geo-bypass";
    system(command.c_str());
    system("youtube-dl --rm-cache-dir");
    return;
}