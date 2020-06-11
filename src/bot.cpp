#include "../include/bot.hpp"

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

bot::bot(){
    memset(&hints,0,sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;
    
    for(int i=0;i<5;i++){
        try{
            bytes_recv = getaddrinfo(serv_addr,port, &hints, &peer_address);
            if(bytes_recv!=0){
                throw "Error trying to reach twitch's DNS\nTrying again in 2 seconds...";
            }
            else break;
        }
        catch(const char* error){
            std::cout << bytes_recv << std::endl;
        }
        sleep(2);
    }
    
    //INITIALIZING SOCKET
    socket_peer = socket(peer_address->ai_family,
    peer_address->ai_socktype, peer_address->ai_protocol);
    if(!ISVALIDSOCKET(socket_peer)){
        std::cout << "socket() failed" << GETSOCKETERRNO() << std::endl;
    }

    //CONNECTION
    if(connect(socket_peer, peer_address->ai_addr, peer_address->ai_addrlen)){
        std::cout << stderr << "connect() failed " << GETSOCKETERRNO() << std::endl;
    }

    //SET TIMEOUT ON CONNECTION
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;

    //STATUP
    srand (time(NULL));
    raffleIsOn = false;
    time(&pingTimer);
    //MODS
    env = new Env();
    login();
    startupIDandFollow();
}
void bot::login(){
    char pass[4096]  = "PASS ";
    char nick[4096] = "NICK ";
    char join[4096] = "JOIN ";

    strcat(pass,(env->getValue("OAUTH")).c_str());
    strcat(pass,"\n");
    strcat(nick,(env->getValue("NICK")).c_str());
    strcat(nick,"\n");
    strcat(join,(env->getValue("CHANNEL")).c_str());
    
    channel = env->getValue("CHANNEL");
    oauth = std::string(env->getValue("OAUTH"));
    privmsg = privmsg + std::string(channel) + std::string(" :");
    
    strcat(join,"\n");

    send(socket_peer,pass,strlen(pass),0);
    send(socket_peer,nick,strlen(nick),0);
    send(socket_peer,join,strlen(join),0);
    recv(socket_peer, NULL, 4096, 0);
}
void bot::pong(){
    char data[4096]  = "PONG :tmi.twitch.tv\n";
    int bytes_sent = send(socket_peer,data,strlen(data),0);
    if(bytes_sent > 1 && devMode) std::cout << "PONG sent\n";
}
//NO CURRENT IMPLEMENTATION
void bot::ping(){
    char data[4096]  = "PING :tmi.twitch.tv\n";
    int bytes_sent = send(socket_peer,data,strlen(data),0);
    if(bytes_sent > 1 && devMode) std::cout << "PING sent\n";
}
void bot::loop(){
    while(true){
        //SETUP READS WITH SOCKET
        FD_ZERO(&reads);
        FD_SET(socket_peer, &reads);
        //WINDOWS
        #if !defined(_WIN32)
            FD_SET(0,&reads);
        #endif
        //CHECK IF SELECT WAS OK
        if(select(socket_peer+1, &reads, 0, 0, &timeout) < 0){
            std::cout << stderr << "select() failed\n" << GETSOCKETERRNO();
        }
        //CHECK PEER SOCKET TO SEE IF ANYTHING WAS RECEIVED
        if(FD_ISSET(socket_peer, &reads)){
            char read[4096];
            bytes_recv = recv(socket_peer, read, 4096, 0);
            //CHECK IF CONNECTION CLOSED
            if(bytes_recv < 1){
                std::cout << "Connection closed by peer\n";
                break;
            }
            //TAKES CARE OF MESSAGE
            numOnBuffer=0;
            readString = read;
            for(int i=0;read[i]!='\0';i++){
                if(read[i]=='\n') numOnBuffer++;
            }
            for(int i = 0;i<numOnBuffer;i++){
                buffer.push_back(readString.substr(0,readString.find_first_of("\n")));
                readString = readString.substr(readString.find_first_of("\n")+1);
            }
            for(auto it = buffer.begin();it!=buffer.end();it++){
                if(devMode)std::cout << "*it = " << *it << std::endl;
                if (!(*it).substr(0,4).compare("PING")) pong();
                else if((*it).find(channel) != std::string::npos){
                    msgCheck(*it);
                }
            }
            buffer.clear();
            //PRINT
            if(devMode){
                std::cout << "Received " << bytes_recv << " bytes, message is: " << std::endl << read << std::endl;
            }
            //CLEAN CHAR ARRAY
            memset(&read, 0, sizeof(read));
        }
        //CHECK PEER_SOCKET TO SEE IF IT HAS ANYTHING TO SEND
        #if defined(_WIN32)
        if(_kbhit()){
        #else
        if(FD_ISSET(0, &reads)){
        #endif
            char read[4096];
            // if(!fgets(read, 4096, stdin)) break;
            if(read[0]=='+'){
                hostCommandChecker(std::string(read));
            }
            else{
                int bytes_sent = send(socket_peer, read, strlen(read), 0);
                if(devMode){
                    std::cout << "Sent " << bytes_sent << " bytes\n";
                }
            }
        }
        checkers();
    }
}

/*
    checks for local host commands
*/
void bot::hostCommandChecker(std::string hostCommand){
    if(hostCommand[hostCommand.size()]=='\n')hostCommand.pop_back();
    if(!hostCommand.compare("+playlist")){
        HCplaylist();
    }
    else if(!hostCommand.compare("+skip")){
        HCskip();
    }
    else if(!hostCommand.compare("+stop") && player.ableToPlay == true){
        HCstop();
    }
    else if(!hostCommand.compare("+play")){
        if(player.ableToPlay == false){
            HCplay();
        }
        else{
            HCstop();
        }
    }
    else if(!hostCommand.compare("+refresh")){
        ncWin.refreshAll();
    }
    else if(!hostCommand.substr(0,4).compare("+vol")){
        HCvolume(hostCommand.substr(5));
    }
    else if(!hostCommand.substr(0,7).compare("+remove")){
        HCremoveFromPlaylist(atoi(hostCommand.substr(7).c_str()));
    }
    else if(!hostCommand.substr(0,12).compare("+changeOrder")){
        HCchangeOrder(hostCommand.substr(13));
    }
    else if(!hostCommand.compare("+viewerCount")){
        HCviewerCount();
    }
    return;
}

void bot::startupIDandFollow(){

    CURL *curl;
    std::string readBuffer;
    Json::CharReaderBuilder rBuilder;
    Json::Value root;
    JSONCPP_STRING errs;
    struct curl_slist* headers = NULL;
    curl = curl_easy_init();
    if(curl) {
        std::string uri = "https://id.twitch.tv/oauth2/validate";
        headers = curl_slist_append(headers, ("Authorization: OAuth " + oauth.substr(6)).c_str() );
        curl_easy_setopt(curl, CURLOPT_URL, uri.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        std::stringstream s(readBuffer);
        Json::parseFromStream(rBuilder, s, &root, &errs);
        clientID = root.get("client_id",root).asString();
        userID = root.get("user_id",root).asString();
    }
}

int bot::HCviewerCount(){
    CURL *curl;
    std::string readBuffer;
    Json::CharReaderBuilder rBuilder;
    Json::Value root; 
    JSONCPP_STRING errs;
    curl = curl_easy_init();
    if(curl) {
        std::string uri = "http://tmi.twitch.tv/group/user/"+ channel.substr(1) +"/chatters";
        curl_easy_setopt(curl, CURLOPT_URL, uri.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        std::stringstream s(readBuffer);
        Json::parseFromStream(rBuilder, s, &root, &errs);
        
        if(!devMode)ncWin.refreshViewerCount(root.get("chatter_count",root).asString(), followCount);
    }
    return -1;
}

static int parseNumber(std::string clientID,std::string userID, Json::Value *name){
    CURL *curl;
    std::string readBuffer;
    Json::CharReaderBuilder rBuilder;
    Json::Value root;
    struct curl_slist* headers = NULL;
    JSONCPP_STRING errs;
    curl = curl_easy_init();
    if(curl) {
        std::string uri = "https://api.twitch.tv/kraken/channels/"+ userID +"/follows";
        headers = curl_slist_append(headers, "Accept: application/vnd.twitchtv.v5+json");
        headers = curl_slist_append(headers, ("Client-ID: " + clientID).c_str());
        curl_easy_setopt(curl, CURLOPT_URL, uri.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);


        std::stringstream s(readBuffer);
        Json::parseFromStream(rBuilder, s, &root, &errs);
        *name = root;
        return root.get("_total",root).asInt();
    }
    return -1;
}

void bot::HCnewFollow(){
    Json::Value name;  
    if(followCount==-1){
        followCount = parseNumber(clientID,userID,&name); 
    }
    else{
        int currentFollowCount = parseNumber(clientID,userID,&name);
        if(followCount < currentFollowCount){
            for(int i = 0; i < currentFollowCount - followCount;i++){
                ncWin.printInfo("New follow:\n" +name.get("follows",name)[i].get("user",name).get("display_name",name).asString(),1);
                followCount = currentFollowCount;
            }
        }
        else if(followCount > currentFollowCount){
            followCount = currentFollowCount; /* :'C */
        }
    }
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp){
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}


void bot::HCchangeOrder(std::string hostCommand){
    auxInt = 0;
    size_t separator = hostCommand.find(' ');
    int toChange = atoi(hostCommand.substr(0,separator).c_str());
    int changeTo = atoi(hostCommand.substr(separator).c_str());
    if(toChange==changeTo) return;
    std::cout << toChange << " " << changeTo << std::endl;
    std::tuple<std::string,std::string> auxTuple;


    for(auto it = player.requestList.begin(); it!=player.requestList.end();auxInt++,it++){
        if(auxInt==toChange){
            auxTuple = *(it);
            auxInt=0;
            for(auto jt = player.requestList.begin(); jt!=player.requestList.end();auxInt++,jt++)
            if(auxInt==changeTo){
                player.requestList.erase(it++);
                player.requestList.insert(jt++,auxTuple);
            }
        }
    }
    
}
void bot::HCplaylist(){
    auxInt = 0;
    for(auto it=player.requestList.begin();it!=player.requestList.end();it++,auxInt++){
        std::cout << auxInt << ":" << std::get<1>(*it) << std::endl;
    }
    return;
}

void bot::HCremoveFromPlaylist(int whichToRemove){
    auxInt=0;
    for(auto it=player.requestList.begin();it!=player.requestList.end();it++,auxInt++){
        if(whichToRemove == auxInt){
            player.requestList.erase(it);
            return;
        }
    }
}

void bot::HCskip(){
    player.vlcSkip();
}
void bot::HCvolume(std::string volume){
    player.vlcChangeVolume(atoi(volume.c_str()));
}
void bot::HCplay(){
    player.ableToPlay = true;
}
void bot::HCstop(){
    player.ableToPlay = false;
}

/*
A few check fucntions to keep track on raffle and vlc player
*/
void bot::checkers(){
    localCommand = ncWin.commandEvent();
    if(localCommand!=""){
        hostCommandChecker(localCommand);
    }
    title = player.checkOnPlayer();
    if(player.state == 3)player.getTime();
    if(title!=""){
        sendprivmsg(std::string("Tocando agora: " + title));
    }
    checkOnRaffle();
    if(difftime(time(NULL),pingTimer) > 4){
        hostCommandChecker("+viewerCount");
        time(&pingTimer);
    }
    ncWin.printPlaylist(player.requestList);
    HCnewFollow();
}

/*
msgCheck(std::string @_msg)
@_msg recv bytes to be parsed and processed
*/
void bot::msgCheck(std::string msgRecv){
    latestMsg = msgManager(msgRecv);
    
    if(latestMsg.text[0] == '!'){
        if(!latestMsg.text.compare("!dice")){
            Cdice(latestMsg);
        }
        else if(!latestMsg.text.compare("!pau")){
            Cdick(latestMsg);
        }
        else if(!latestMsg.text.compare("!playlist")){
            Cplaylist(latestMsg);
        }
        else if(!latestMsg.text.substr(0,4).compare("!add") || !latestMsg.text.substr(0,5).compare("!play") || !latestMsg.text.substr(0,3).compare("!sr")){
            Crequest(latestMsg);
        }
        else if(!latestMsg.text.substr(0,4).compare("!raf")){
            Craffle(latestMsg);
        }
    }
    /*
        Fazer classe Comando com função abstrata @override executar
        cada classe tera um string pra comparação
    */    
    return;
}

/*
msgManaget(std::string @_s)
@_s unfiltered byte stream
returns struct msg with user and text filtered
*/
struct msg bot::msgManager(std::string msgRecv){
    latestMsg.user = msgRecv.substr(1,std::string(msgRecv).find('!')-1);
    latestMsg.text = msgRecv.substr(std::string(msgRecv).find(" :")+2);
    ncWin.printChat(latestMsg.user + ": " + latestMsg.text);
    if(chatMode) std::cout << latestMsg.user <<" : " << latestMsg.text << std::endl;
    //string fix
    latestMsg.text.pop_back();
    return latestMsg;
}
/*
isAdm(std::string @_s)
@_s is user to be checked
returns true if on mods list
else returns false
*/
bool bot::isAdm(std::string user){
    for(auto i = mods.begin();i!=mods.end();i++){
        if(!(i->compare(user))){
            return true;
        }
    }
    return false;
}


void bot::Cdice(struct msg latestMsg){
    msg ="@" + latestMsg.user + " rolled " +
                        std::to_string((rand()%20)+1);
    ncWin.printInfo(msg,0);
    sendprivmsg(msg);
}
void bot::Cdick(struct msg latestMsg){
    msg ="@" + latestMsg.user + " tem " +
                            std::to_string((rand()%20)+1) + "cm de pinto";
    ncWin.printInfo(msg,0);
    sendprivmsg(msg);
}
void bot::Craffle(struct msg latestMsg){
    if(isAdm(latestMsg.user)){
        if(!raffleIsOn){
            time(&raffleTimer);
            raffleIsOn = true;
            raffleSeconds = atoi(latestMsg.text.substr(4).c_str());
            if(raffleSeconds>120){
                raffleSeconds=120;
            }
        }
        else{
            raffleList.push_back(latestMsg.user);
        }
    }
}
void bot::Crequest(struct msg latestMsg){
    int requestResponse = player.addToRequestList(latestMsg.text.substr(4));
    switch (requestResponse){
    case 1:
        msg = "@" + latestMsg.user + " Seu link foi negado pelo bot, por favor não usar link de radio, obrigado!";
        break;
    case 2:
        msg = "@" + latestMsg.user + " Esta música já foi adicionada na playlist, fica esperto que daqui a pouco toca jpelizMK";
        break;
    default:
        msg = "@" + latestMsg.user + " Música adicionada a playlist sucesso!!";
        ncWin.printPlaylist(player.requestList);
        ncWin.printInfo(latestMsg.user + " added: \n" +player.title.substr(0,(2*ncWin.col/5)-4) + "\nto the playlist",0);
        break;
    }
    sendprivmsg(msg);
    return;
}
void bot::Cplaylist(struct msg latestMsg){
    auxInt=0;
    if(player.requestList.size()>0){
        msg = "Up next: ";
        for(auto it=player.requestList.begin();it!=player.requestList.end() && auxInt<3;it++,auxInt++){
            msg += std::get<1>(*it) + " ; ";
        }
        if((int(player.requestList.size()) - 3) > 0){
            msg += " e mais "+ std::to_string(player.requestList.size()-std::size_t(3)) + " outras!";
        }
    }
    else{
        msg = "A playlist esta vazia, adiciona uma música com: !add (<yt-link>|nome da música)";
    }
    sendprivmsg(msg);
    return;
}

/*
checkOnRaffle()
If raffle is running, keeps track of time and users participating
*/
void bot::checkOnRaffle(){
    if(raffleIsOn==false){
        return;
    }
    else if(difftime(time(NULL),raffleTimer) > raffleSeconds){
        std::cout << raffleList.size() << std::endl;
        raffleIsOn = false;

        if(raffleList.size() < 1) return;

        msg = "@" + raffleList.at(rand()%raffleList.size()) + " won the raffle! CONGRATULATIONS!";

        sendprivmsg(msg);

        while(raffleList.size() > 0){
            raffleList.erase(raffleList.begin());
        }
    }
}
/*
sendprivmsg(std::string @_s)
sends @_s and privsmg on connected channel
*/
void bot::sendprivmsg(std::string text){
    text = privmsg + text + "\n";
    int bytes_sent = send(socket_peer,text.c_str(),strlen(text.c_str()),0);
    if(bytes_sent<1){
        std::cout << "ERROR TRYING TO SEND MESSAGE\n";
    }
    text.clear();
}