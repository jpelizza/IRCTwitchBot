#ifndef ENV_H
#define ENV_H

#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>

class Env{

public:
    std::vector<std::pair<std::string,std::string>> variables{
        *new std::pair<std::string, std::string>("OAUTH",""),
        *new std::pair<std::string, std::string>("NICK",""),
        *new std::pair<std::string, std::string>("CHANNEL",""),
        *new std::pair<std::string, std::string>("DEBUG_MODE","")
    };

    Env();
    bool fileExists();
    void parseFile();
    bool setKey(std::string key, std::string value);
    void hasEmptyValue();
    std::string getValue(std::string key);
};

#endif