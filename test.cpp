#include <iostream>
#include <time.h>
#include <unistd.h>

int main(void){

    time_t now;
    time(&now);
    sleep(10);
    int seconds = difftime(time(NULL),now);

    std::cout << seconds << std::endl;

    return 0;
}