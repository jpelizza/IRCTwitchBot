#include "../include/main.hpp"

int main(int argc, char *argv[]){
    setlocale(LC_ALL, "");
    bot t = bot();
    t.loop();
    endwin();
    return 0;
}