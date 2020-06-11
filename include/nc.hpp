#ifndef NC
#define NC

#include <ncurses.h>
#include <vector>
#include <list>
#include <tuple>
#include <iostream>
#include <string>

struct ncColor{
    bool color,change_color;
    std::vector<int> color_pair;
};

class ncBotWindow{
    public:
    WINDOW *wib, *wcb, *wpb, *wsb, *wbb;
    std::string borderHeaders[5] = {
        "INFO",
        "STATISTICS",
        "CHAT",
        "PLAYLIST",
        "COMMANDS"
    };
    WINDOW *winfo,*wchat,*wpl, *wbutton, *wstatistics;
    MEVENT m_event;
    int key = 0;
    std::vector<WINDOW*> ncAllWindow,ncWindowList,ncBorderList;
    int row,col;

    public:
    ncBotWindow();
    WINDOW *create_newwin(int height, int width, int starty, int startx, bool tbox, int color);
    void create_layout();
    void layout_buttons();
    void printChat(std::string);
    void printInfo(std::string,int color);
    void printPlaylist(std::list<std::tuple<std::string,std::string>>);
    std::string commandEvent();
    void refreshAll();
    void refreshViewerCount(std::string,int);
    void startup();
};

void ncursesStartup();
void init_ncc();
void printChat(std::string);
WINDOW *create_newwin(int,int,int,int,bool, int);


#endif