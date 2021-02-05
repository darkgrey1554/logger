
#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <list>
#include <string> 
#include <exception>
#include <thread>
#include <mutex>

#ifdef _WIN32
#include <windows.h>
#pragma warning(disable : 4996)
#endif // _WIN32

#ifdef __linux__
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdio.h>
#include <syslog.h>
#endif // __linux

namespace LoggerSpace
{
    enum class LogMode
    {
        DEBUG = 0,
        INFO = 1,
        WARNING = 2,
        ERR = 3
    };

    /// enum for ON OFF thread of writer
    enum class Status
    {
        OFF,
        ON
    };

    /// struct messeng of System log
    struct MessengSysLog
    {
        std::string messeng;
        int status;

        void clear()
        {
            messeng.clear();
            status = 0;
        };
    };


    class Logger
    {
    private:

        Status AskStatusLog = Status::OFF;  // mark work thread
        Status StatusLog = Status::OFF; // status work logger
        //char flag_end = 0;       //  stream end mark
        char initthread = 0;
        int count_file = 0;      // current number of logfile 
        char work_list = 0;      // current numder work list
        std::thread RiverWrite;  // log file stream
        std::mutex event_write;  // mutex for chek work thread
        std::mutex mutex_turn_log;  // mutex for chek work thread
        std::mutex event_protect_WrDel_list; // mutex for sheet replacement
        std::list <std::string> LogList[2];  // lists messengs
        std::string NameFile;                // name log file for open file
        std::string NameLog;                 // name log file

        int SizeLogFile = 1 * 1024 * 1024;    // max size log file
        LogMode ModeLog = LogMode::DEBUG;     // mode  logger
        int DayWrite = 1;                     // maximum duration of file recording (days)




        Status AskStatusSysLog = Status::OFF;
        std::mutex mutex_turn_syslog;
        char initsysthread = 0;
        //char flag_sys_end = 0;
        char work_syslist = 0;
        std::thread RiverSysWrite;
        Status StatusSysLog = Status::OFF;
        std::list <MessengSysLog> LogSysList[2];
        std::mutex event_protect_WrDel_Syslist;
        std::mutex event_sys_write;
        std::string NameSysLog;
        LogMode ModeSysLog = LogMode::DEBUG;

#ifdef _WIN32
        SYSTEMTIME t_last;
        HANDLE handelsyslog = NULL; // point contact with winlog
#endif // _WIN32       
#ifdef __linux__
        timeval t_last;
#endif // __linux__


        int ThreadWriteLog();   // thread write 
        int ThreadSysWriteLog();
        void WriteLogMesseng(LoggerSpace::LogMode current_mode, const char** form); // func write in log list
        void WriteLogSysMesseng(LoggerSpace::LogMode current_mode, const char** form);
        std::string take_log_name(std::string first_str, int count = -1);


        // --- pattern singeton --- //
        void operator=(const Logger&) = delete;
        Logger() = delete;
        Logger(const char* str);
        ~Logger();
        static Logger* p_contact;
        static std::mutex MutLogInit;

    public:



        static Logger* getpointcontact(const char* NameLog = "log"); // access to class

        void SetSizeFile(int baits);  // get set max size log-file
        int GetSizeFile();

        void SetLogMode(LoggerSpace::LogMode mode); // get set mode log
        LoggerSpace::LogMode GetLogMode();
        void SetSysLogMode(LoggerSpace::LogMode mode);
        LoggerSpace::LogMode GetSysLogMode();

        void SetNameLog(const char* str);  // get set name log
        void SetNameSysLog(const char* str);

        void TurnOnLog(); // turnon turnoff write to file
        void TurnOffLog();
        void TurnOnSysLog();
        void TurnOffSysLog();

        LoggerSpace::Status GetStatusSysLog(); // get set status thread of log
        LoggerSpace::Status GetStatusLog();

        void WriteLogDEBUG(const char* form);  // func write log file
        void WriteLogINFO(const char* form);
        void WriteLogWARNING(const char* form);
        void WriteLogERR(const char* form);

        void WriteLogDEBUG(const char* form, int code_error, int syscode_error);
        void WriteLogINFO(const char* form, int code_error, int syscode_error);
        void WriteLogWARNING(const char* form, int code_error, int syscode_error);
        void WriteLogERR(const char* form, int code_error, int syscode_error);
    };
}

#endif 