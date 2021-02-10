#include <iostream>
#include "logger.h"
#include <thread>

void thread_1()
{
    LoggerSpace::Logger* log = LoggerSpace::Logger::getpointcontact();

    int i = 0;
    for (;;)
    {
        log->WriteLogINFO("MESSENG THREAD_1");
        log->WriteLogWARNING("MESSENG THREAD_1", 0, 0);
        i++;
        if (i > 10) break;
        Sleep(100);
    }

    return;
}

void thread_2()
{
    LoggerSpace::Logger* log = LoggerSpace::Logger::getpointcontact();

    int i = 0;
    for (;;)
    {
        log->WriteLogINFO("MESSENG THREAD_2");
        log->WriteLogWARNING("MESSENG THREAD_2", 0, 0);
        i++;
        if (i > 10) break;
        Sleep(110);
    }

    return;
}

int main()
{
    LoggerSpace::Logger* log = LoggerSpace::Logger::getpointcontact();

    log->SetLogMode(LoggerSpace::LogMode::INFO);
    log->SetNameLog("LOG");
    log->SetSizeFile(100000);
    log->TurnOnLog();

    std::thread t1(thread_1);
    std::thread t2(thread_2);
    Sleep(10);

    if (t1.joinable()) t1.join();
    if (t2.joinable()) t2.join();

    return 0;

}


