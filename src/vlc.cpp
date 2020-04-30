#include "../include/vlc.hpp"

vlc::vlc(){
    inst = libvlc_new (0, NULL);
    state= -1;
    cont = 0;
    ableToPlay = false;
    getStandByPlaylist();
    srand(time(NULL));
}
/*
addToRequestList(std::string @_s)
returns -1 if successful
1 if regex does not identify link
2 if link alredy on playlist
*/
int vlc::addToRequestList(std::string url){
    
    regex_match(url.c_str(),c,std::regex("(?:(?:.*v=([^&]*).*))|(?:.*be/(.*))"));
    std::cmatch ccopy(c);

    for(unsigned i=0; i<ccopy.size(); i++){
        if(ccopy[i].length()==11) url = ccopy[i];
    }

    //if not URL
    if(url.size()!=11){
        url = searchByName(url);
    }
    //else if URL
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
    myfile.close();
    requestList.push_back(std::make_tuple(url,title));
    return -1;
}

std::string vlc::searchByName(std::string url){
    std::string search;
    system(std::string("youtube-dl ytsearch:\"" + url + "\" --get-id > title.txt").c_str());
    myfile.open("title.txt", std::ios::out | std::ios::app | std::ios::binary);
    getline(myfile,search);
    myfile.close();
    return search;
}

/*
getTitle(std::string @_s)
requests youtube-dl for a title then streams it to title.txt
*/
void vlc::getTitle(std::string url){
    system(std::string("youtube-dl " + url + " --get-title > title.txt").c_str());
    return;
}
/*
std::string @_t checkOnPlayer()
check if vlc is running, keeps playlist on track and returns title
*/
std::string vlc::checkOnPlayer(){
    
    if(state!=-1)state = libvlc_media_player_get_state(mp);

    if(state == 0 || state == 1 || state == 3)return "";
    else if( requestList.size() > 0 && ableToPlay) {
        vlcPlay(std::get<0>(requestList.front()));
        title = std::get<1>(requestList.front());
        requestList.pop_front();
        return title;
    }
    else if(requestList.size() <= 0 && ableToPlay){
        addToRequestList(standByPlaylist.front());
        standByPlaylist.push_back(standByPlaylist.front());
        standByPlaylist.erase(standByPlaylist.begin());
        if(cont < 5){
            cont++;
            return "";
        }
        else{
            cont = 0;
            return "Stand by playlist, você pode adicionar uma música usando: !add (<yt-link>|nome_da_música)";
        }
    }
    return "";
}
/*
vlcPlay(std::string @_s)
searches for song on ./music/
if after 5 seconds the file is not found moves
*/
void vlc::vlcPlay(std::string url){

    for(int i=0;(!exists(std::string("./music/") + url) && i<5);i++){
        sleep(2);
    }
    #if defined(_WIN32)
    if(!exists(std::string(".\\music\\")+url)){
        return;
    }
    m = libvlc_media_new_path (inst, (std::string(".\\music\\")+ url).c_str());
    #else
    if(!exists(std::string("./music/") + url)){
        return;
    }
    m = libvlc_media_new_path (inst, (std::string("./music/") + url).c_str());
    #endif
    mp = libvlc_media_player_new_from_media (m);
    libvlc_media_release (m);
    libvlc_media_player_play (mp);
    state = libvlc_media_player_get_state(mp);
    return;
}
/*
Stops VLC player skipping music
*/
void vlc::vlcSkip(){
    if(state==-1){return;}
    libvlc_media_player_stop(mp);
    return;
}
/*
changes vlc instance volume
*/
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
    #if defined(_WIN32)
    std::string command = "youtube-dl -f 'bestaudio[filesize<10M]' -o .\\music\\"+url+" "+"\""+url+"\""+" --no-playlist --geo-bypass --no-cache-dir > dwnloadDebug.txt";
    #else
    std::string command = "youtube-dl -f 'bestaudio[filesize<10M]' -o ./music/"+url+" "+"\""+url+"\""+" --no-playlist --geo-bypass --no-cache-dir > dwnloadDebug.txt";
    #endif
    system(command.c_str());
    return;
}
void vlc::getStandByPlaylist(){
    std::ifstream file("standByPlaylist.txt");
    std::string str;
    std::string file_contents;
    while (std::getline(file, str)){
        standByPlaylist.push_back(str);
    }
    std::mt19937 g(rd());
    std::shuffle(standByPlaylist.begin(),standByPlaylist.end(),g);
}
