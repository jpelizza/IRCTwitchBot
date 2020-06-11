#include "../include/nc.hpp"



ncBotWindow::ncBotWindow(){
    initscr();
    noecho();
    nocbreak();
    halfdelay(2);
    use_default_colors();
    mousemask(ALL_MOUSE_EVENTS,NULL);
    start_color();
    curs_set(0);
    getmaxyx(stdscr,this->row,this->col);
    create_layout();
}

std::string ncBotWindow::commandEvent(){
    key = wgetch(wbutton);
    if(key != ERR) {
            if(getmouse(&m_event)==OK){
                if(m_event.bstate && BUTTON1_PRESSED &&
                m_event.x>= wbutton->_begx && m_event.x<= wbutton->_begx+9 &&
                m_event.y== wbutton->_begy){
                    printInfo("PLAY/STOP!",0);
                    return "+play";
                }
                else if(m_event.bstate && BUTTON1_PRESSED &&
                m_event.x >= wbutton->_begx + 10 && m_event.x <= wbutton->_begx+15 &&
                m_event.y == wbutton->_begy){
                    printInfo("SKIP!",0);
                    return "+skip";
                }
                else if(m_event.bstate && BUTTON1_PRESSED &&
                m_event.y == wbutton->_begy + wbutton->_maxy){
                    if(m_event.x - wbutton->_begx > 1 && m_event.x - wbutton->_begx < 22){
                        int new_vol = (m_event.x - wbutton->_begx)*5;
                        std::string s("VOL" + std::to_string(new_vol));
                        printInfo(s,0);
                        return "+vol " +std::to_string(new_vol);
                    }
                }
                else if(m_event.bstate && BUTTON1_PRESSED &&
                m_event.x  > wbutton->_maxx && m_event.y == wbutton->_begy){
                    printInfo("REFRESH!",0);
                    return "+refresh";
                }
            }
    }
    return "";
}

void ncBotWindow::refreshAll(){
    for(auto it=ncWindowList.begin();it!=ncWindowList.end();it++){
        wrefresh(*it);
    }
}

WINDOW *ncBotWindow::create_newwin(int height, int width, int starty, int startx, bool tbox, int color){
    WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
    

    if(color != -1){
        wattron(local_win,COLOR_PAIR(color));
    }
	if(tbox){
        box(local_win, 0 , 0);
        this->ncBorderList.push_back(local_win);
    }
    else
    {
        this->ncWindowList.push_back(local_win);
    }
    
	wrefresh(local_win);		/* Show that box 		*/
    this->ncAllWindow.push_back(local_win);
	return local_win;
}

void ncBotWindow::create_layout(){
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_BLACK);
    init_pair(5, COLOR_CYAN, COLOR_BLACK);
    init_pair(6, COLOR_GREEN, COLOR_BLACK);
    init_pair(7,COLOR_BLUE,COLOR_BLACK);

    wib = create_newwin(4*row/5,col/5,0,0,true,1);
    wsb = create_newwin(row/5+1,col/5,4*row/5,0,true,2);
    wcb = create_newwin(row,3*col/5,0,col/5,true,3);
    wpb = create_newwin(4*row/5,col/5,0,4*col/5,true,4);
    wbb = create_newwin(row/5+1,col/5,4*row/5,4*col/5,true,5);

    auto it = ncBorderList.begin();
    for(int i=1;it!=ncBorderList.end();it++,i++){
        wattron((*it),COLOR_PAIR(i));
        wattron((*it),A_BOLD);
    }


    it = ncBorderList.begin();
    for(int i=1;it!=ncBorderList.end();it++,i++){
        mvwprintw((*it),0,((*it)->_maxx/2)-2,borderHeaders[i-1].c_str());
        wattroff((*it),COLOR_PAIR(i));
        wattroff((*it),A_BOLD);
    }

    winfo = create_newwin((4*row/5)-2,(col/5)-2,1,1,false,-1);
    wstatistics = create_newwin(row/5-2,col/5-2,4*row/5+1,1,false,-1);
    wchat = create_newwin(row-2,(3*col/5)-2,1,(col/5)+1,false,-1);
    wpl = create_newwin((4*row/5)-2,(col/5)-2,1,(4*col/5)+1,false,-1);
    wbutton = create_newwin(row/5-2,col/5-2,4*row/5+1,4*col/5+1,false,-1);
    layout_buttons();


    for(auto it = ncAllWindow.begin();it!=ncAllWindow.end();it++){
        wrefresh(*it);
    }
    for(auto it = ncWindowList.begin();it!=ncWindowList.end();it++){
        scrollok(*it,true);
    }
    wattron(wchat,COLOR_PAIR(6));
    keypad(wbutton,true);
    wmove(wchat,0,0);
    return;    
}

void ncBotWindow::printChat(std::string msg){
    if(wchat->_cury>wchat->_maxy-2){
        wscrl(wchat,1);
    }
    wattron(wchat,A_BOLD);
    wprintw(wchat,msg.substr(0,msg.find(":")).c_str());
    wattroff(wchat,A_BOLD);
    wprintw(wchat,msg.substr(msg.find(":")).c_str());
    wmove(wchat,wchat->_cury+1,0);
    wrefresh(wchat);
    return;
}

void ncBotWindow::printInfo(std::string msg,int color){
    wattron(winfo,A_BOLD);
    if(color==1){
        wattron(winfo,COLOR_PAIR(7));
    }
    wprintw(winfo,(msg+"\n").c_str());
    wattroff(winfo,A_BOLD);
    wrefresh(winfo);
    if(color==1){
        wattroff(winfo,COLOR_PAIR(7));
    }
    return;
}

void ncBotWindow::printPlaylist(std::list<std::tuple<std::string,std::string>> list){
    int aux=1;
    wclear(wpl);
    for(auto it = list.begin();it!=list.end();it++,aux++){
        wprintw(wpl,std::string(std::to_string(aux) + "-" + (std::get<1>(*it)).substr(0,(wpl->_maxx*2)-2)).c_str());
        wmove(wpl,wpl->_cury+1,0);
    }
    wrefresh(wpl);
    return;
}

void ncBotWindow::layout_buttons(){
    mvwprintw(wbutton,0,0,"PLAY/STOP");
    mvwprintw(wbutton,0,12,"SKIP");
    mvwprintw(wbutton,0,wbutton->_maxx-1,"R");
    for(int i=0;i<10;i++){
        mvwprintw(wbutton,(wbutton->_maxy),1+(2*i),std::to_string(i).c_str());
    }
    return;
}
void ncBotWindow::refreshViewerCount(std::string num, int followCount){
    std::string v = "Viewers: " + num + "\nFollows: "+ std::to_string(followCount);
    wclear(wstatistics);
    wprintw(wstatistics,v.c_str());
    wrefresh(wstatistics);
}