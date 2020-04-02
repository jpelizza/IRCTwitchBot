#include "../../include/env.hpp"

Env::Env(){
    if(!this->fileExists()){
        std::cerr<<"!>.env file do not exist!"<<std::endl;
        exit(1);
    }
    this->parseFile();
    this->hasEmptyValue();
    
    //PRINTS KEY BE CAREFUL!

    // for(auto k: this->variables){
    //     std::cout<<">>"<<k.first<<"->"<<k.second<<std::endl;
    // }

}

bool Env::fileExists(){
    if (FILE *file = fopen(".env", "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }  
}


void Env::parseFile(){
    std::ifstream envFile (".env");
    std::string line;
    int lineNumber = 1;
    while(getline(envFile,line)){
        line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());

        if(line[0] == '#' || line.empty())
            continue;

        auto delimiter = line.find("=");
        if (delimiter==std::string::npos){
            std::cerr<<"!> .env syntax error in line "<<lineNumber<<std::endl;
            exit(1);
        }

        std::string key = line.substr(0,delimiter);
        std::string value = line.substr(delimiter+1);

        if(!this->setKey(key, value)){
            std::cerr<<"!> Key \""<<key<<"\" in line "<<lineNumber<<" does not exist!"<<std::endl;
            exit(1);
        }

        lineNumber++;
    }
}

bool Env::setKey(std::string key, std::string value){
    for(auto &k: this->variables){
        if(k.first.compare(key)==0){
            k.second = value;
            return true;
        }
    }

    return false;
}

void Env::hasEmptyValue(){
    for(auto k: this->variables){
        if(k.second.compare("")==0){
            std::cerr<<"!> Key \""<<k.first<<" was not set!"<<std::endl;
            exit(1);
        }
    }
}

std::string Env::getValue(std::string key){
    for(auto k: this->variables){
        if(!k.first.compare(key)){
            return k.second;
        }
    }
    std::cerr<<"!> Key \""<<key<<" does not exist!"<<std::endl;
    exit(1);
}