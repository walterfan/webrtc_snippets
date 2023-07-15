#include <stdio.h>
#include <stdint.h>
#include <string>
#include <map>
#include <iostream>
#include <memory>



#ifndef DEBUG_TRACE
#define DEBUG_TRACE(msg) do { \
            std::cout <<"["<<time(NULL)<<","<< __FILE__ << "," << __LINE__ << "]\t"<< msg << std::endl; \
        } while(0)
#endif

using namespace std;


int main(int argc, char** argv)
{

    DEBUG_TRACE("start");
    return 0;
}