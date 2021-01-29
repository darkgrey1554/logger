
#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <list>
#include <string> 
#include <exception>
#include <thread>
#include <mutex>


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


    class Logger
    {
    private:

        Status AskStatusLog = Status::OFF;  // mark work thread
        Status StatusLog = Status::OFF; // status work logger
      
        std::string NameFile;                // name log file for open file
        std::string NameLog;                 // name log file
        
        int SizeLogFile = 1 * 1024 * 1024;    // max size log file
        LogMode ModeLog = LogMode::DEBUG;     // mode  logger
        int DayWrite = 1;                     // maximum duration of file recording (days)
       
        
        
      
        LogMode ModeSysLog = LogMode::DEBUG;


        int ThreadWriteLog();   // thread write 
        int ThreadSysWriteLog();
        void WriteLogMesseng(LoggerSpace::LogMode current_mode,const char** form); // func write in log list
        void WriteLogSysMesseng(LoggerSpace::LogMode current_mode, const char** form);
        std::string take_log_name(std::string first_str, int count = -1);


        // --- pattern singeton --- //
        void operator=(const Logger&) = delete;
        Logger() = delete;
        Logger(const char* str);
        ~Logger(); 
        static Logger* p_contact;  
            
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