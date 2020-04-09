#include "../include/bot.hpp"


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
    
    //MODS
    mods.push_back("jpelizza");
    mods.push_back("uneaseplacebo");
    mods.push_back("ricardostoklosa");
    mods.push_back("tteknahlowg");
    mods.push_back("manakithegreat");
    mods.push_back("alucard27xxx");
    mods.push_back("utechhh");
    
    
    login();
}

void bot::login(){
    Env *env = new Env();

    char pass[4096]  = "PASS ";
    char nick[4096] = "NICK ";
    char join[4096] = "JOIN ";

    strcat(pass,(env->getValue("OAUTH")).c_str());
    strcat(pass,"\n");
    strcat(nick,(env->getValue("NICK")).c_str());
    strcat(nick,"\n");
    strcat(join,(env->getValue("CHANNEL")).c_str());
    
    channel = env->getValue("CHANNEL");
    privmsg = privmsg + std::string(channel) + std::string(" :");
    
    strcat(join,"\n");

    send(socket_peer,pass,strlen(pass),0);
    send(socket_peer,nick,strlen(nick),0);
    send(socket_peer,join,strlen(join),0);
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
            //TAKES CARE OF MESSAGE
            if (!std::string(read).substr(0,4).compare("PING")) pong();
            else if(std::string(read).find(channel) != std::string::npos){
                msgCheck(read);
            }
            else{
                std::cout<<this->channel<<std::endl;
            }
            //CHECK IF CONNECTION CLOSED
            if(bytes_recv < 1){
                std::cout << "Connection closed by peer\n";
                break;
            }
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
            if(!fgets(read, 4096, stdin)) break;
            int bytes_sent = send(socket_peer, read, strlen(read), 0);
            if(devMode){
                std::cout << "Sent " << bytes_sent << " bytes\n";
            }
        }
        player.checkOnPlayer();
        checkOnRaffle();
    }
}
void bot::msgCheck(char *msgRecv){
    struct msg latestMsg = msgManager(msgRecv);

    if(latestMsg.text[0] == '!'){
        if(!latestMsg.text.compare("!dice")){
            Cdice(latestMsg);
        }
        else if(!latestMsg.text.compare("!pau")){
            Cdick(latestMsg);
        }
        else if(!latestMsg.text.compare("!gamer")){
            Cgamer(latestMsg);
        }
        else if(!latestMsg.text.compare("!skip")){
            Cskip(latestMsg);
        }
        else if(!latestMsg.text.compare("!stop")){
            Cstop(latestMsg);
        }
        else if(!latestMsg.text.compare("!play")){
            Cplay(latestMsg);
        }
        else if(!latestMsg.text.substr(0,4).compare("!add")){
            Crequest(latestMsg);
        }
        else if(!latestMsg.text.substr(0,4).compare("!raf")){
            Craffle(latestMsg);
        }
        else if(!latestMsg.text.substr(0,4).compare("!vol")){
            Cvolume(latestMsg);
        }
    }
    return;
}
struct msg bot::msgManager(char *msgRecv){
    if(devMode) std::cout << "msgManager()\n";

    latestMsg.user = std::string(msgRecv).substr(1,std::string(msgRecv).find('!')-1);
    latestMsg.text = std::string(msgRecv).substr(std::string(msgRecv).find(" :")+2);
    if(chatMode) std::cout << latestMsg.user <<" : " << latestMsg.text << std::endl;
    //string fix
    latestMsg.text.pop_back();latestMsg.text.pop_back();
    
    return latestMsg;

}
bool bot::isAdm(std::string user){
    for(auto i = mods.begin();i!=mods.end();i++){
        if(!(i->compare(user))){
            return true;
        }
    }
    return false;
}
void bot::Cgamer(struct msg latestMsg){
    msg = privmsg + "Ser gamer, ser um jogador, vocês já se questionaram o que é ser um gamer? Já se questionaram o que os videogames te ensinaram pra você levar pra sua vida? ou você nunca parou pra pensar nisso? afinal são horas e horas que dedicamos a eles, muitas horas de nossos dias e nossas vidas. Se vocês forem bons observadores, vão notar que eles tem muitas coisas pra nos ensinar, nos inspirar. Zangado, o que os games ensinaram a voce? O que é ser um gamer? Eu digo a vocês. Os games me ensinaram que quando";
    std::cout << msg << std::endl;
    sendprivmsg(msg);
}
void bot::Cdice(struct msg latestMsg){
    msg = privmsg + "@" + latestMsg.user + " rolled " +
                        std::to_string((rand()%20)+1);
    std::cout << msg << std::endl;
    sendprivmsg(msg);
}
void bot::Cdick(struct msg latestMsg){
    msg = privmsg + "@" + latestMsg.user + " tem " +
                            std::to_string((rand()%20)+1) + "cm de pinto";
    sendprivmsg(msg);
}
void bot::Craffle(struct msg latestMsg){
    if(isAdm(latestMsg.user)){
        if(!raffleIsOn){
            time(&raffleTimer);
            raffleIsOn = true;
            raffleSeconds = atoi(latestMsg.text.substr(4).c_str());
            if(raffleSeconds>600){
                raffleSeconds=600;
            }
        }
        else{
            raffleList.push_back(latestMsg.user);
        }
    }
}
void bot::Crequest(struct msg latestMsg){
    if(!player.addToRequestList(latestMsg.text.substr(4))){
        msg = privmsg + "@" + latestMsg.user + " Seu link foi negado pelo bot, por favor não usar playlist ou radio, obrigado!";
        sendprivmsg(msg);
    }
    return;
}
void bot::Cskip(struct msg latestMsg){
    if(latestMsg.user.find(channel) && isAdm(latestMsg.user)){
        player.vlcSkip();
    }
}
void bot::Cvolume(struct msg latestMsg){
    if(latestMsg.user.find(channel) && isAdm(latestMsg.user)){
        player.vlcChangeVolume(atoi(latestMsg.text.substr(4).c_str()));
    }
}
void bot::Cplay(struct msg latestMsg){
    if(latestMsg.user.find(channel) && isAdm(latestMsg.user)){
        player.ableToPlay = true;
    }
}
void bot::Cstop(struct msg latestMsg){
    if(latestMsg.user.find(channel) && isAdm(latestMsg.user)){
        player.ableToPlay = false;
    }
}
void bot::checkOnRaffle(){
    
    if(raffleIsOn==false){
        return;
    }

    else if(difftime(time(NULL),raffleTimer) > raffleSeconds){
        raffleIsOn = false;

        if(raffleList.size() < 1) return;

        msg = privmsg + std::string(channel) + " :@" +
                            raffleList.at(rand()%raffleList.size()) +
                            " won the raffle! CONGRATULATIONS!";

        sendprivmsg(msg);

        while(raffleList.size() > 0){
            raffleList.erase(raffleList.begin());
        }
    }
}
void bot::sendprivmsg(std::string text){
    text = text + "\n";
    int bytes_sent = send(socket_peer,text.c_str(),strlen(text.c_str()),0);
    if(bytes_sent<1){
        std::cout << "ERROR TRYING TO SEND MESSAGE\n";
    }
}