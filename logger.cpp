#include "logger.h"


namespace LoggerSpace
{

    /// --- SINGELTON ---///
    Logger* Logger::p_contact = nullptr;
    std::mutex Logger::MutLogInit;
    Logger* Logger::getpointcontact(const char* NameLog)

    {
        if (p_contact == nullptr)
        {
            std::lock_guard<std::mutex> lock(MutLogInit);
            if (p_contact == nullptr)
            {
                p_contact = new Logger(NameLog);
            }

        }
        return p_contact;
    }

    /// --- INIT LOG CLASS --- ///
    Logger::Logger(const char* str)
    {
        /// --- set name log -- //
        NameLog.clear();
        NameLog = str;
        NameSysLog.clear();
        NameSysLog = str;

        /// --- finder last log file -- //
        std::string namefile;
        count_file = 0;
        NameFile = take_log_name(NameLog);
#ifdef _WIN32
        HANDLE init_count_file = INVALID_HANDLE_VALUE;
        GetLocalTime(&t_last);
        do
        {
            namefile = take_log_name(NameLog, count_file);
            init_count_file = CreateFileA(namefile.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH, NULL);
            if (init_count_file != INVALID_HANDLE_VALUE) count_file++;
            CloseHandle(init_count_file);
        } while (init_count_file != INVALID_HANDLE_VALUE);
#endif
#ifdef __linux__
        int init_count_file;
        gettimeofday(&t_last, NULL);
        do
        {
            namefile = take_log_name(NameLog, count_file);
            init_count_file = open(namefile.c_str(), O_RDONLY, O_APPEND);
            if (init_count_file != -1) count_file++;
            close(init_count_file);
        } while (init_count_file != -1);
#endif

    }


    void Logger::TurnOnLog()
    {
        mutex_turn_log.lock();
        if (AskStatusLog == Status::OFF)
        {
            initthread = 1;
            AskStatusLog = Status::ON;
            RiverWrite = std::thread(&Logger::ThreadWriteLog, this);
#ifdef _WIN32
            Sleep(1);
#endif
#ifdef __linux__
            usleep(1000);
#endif
            //StatusLog = Status::ON;   
        }
        mutex_turn_log.unlock();
    }

    void Logger::TurnOnSysLog()
    {
        mutex_turn_syslog.lock();
        if (AskStatusSysLog == Status::OFF)
        {
            initsysthread = 1;
            AskStatusSysLog = Status::ON;
            RiverSysWrite = std::thread(&Logger::ThreadSysWriteLog, this);
#ifdef _WIN32
            Sleep(1);
#endif
#ifdef __linux__
            usleep(1000);
#endif
            //StatusSysLog = Status::ON;            
        }
        mutex_turn_syslog.unlock();
    }

    void Logger::TurnOffLog()
    {
        mutex_turn_log.lock();
        if (StatusLog == Status::ON)
        {
            AskStatusLog = Status::OFF;
        }
        mutex_turn_log.unlock();
        if (RiverWrite.joinable()) RiverWrite.join();
    }

    void Logger::TurnOffSysLog()
    {
        mutex_turn_syslog.lock();
        if (StatusSysLog == Status::ON)
        {
            AskStatusSysLog = Status::OFF;
        }
        mutex_turn_syslog.unlock();
        if (RiverSysWrite.joinable()) RiverSysWrite.join();
    }



    void Logger::WriteLogMesseng(LoggerSpace::LogMode current_mode, const char** form)
    {
#ifdef _WIN32
        SYSTEMTIME st;
#endif
#ifdef __linux__
        tm* st;
        timeval stmsec;
#endif
        std::string helpstr;
        std::string time;
        char c[25];
        try
        {
            /// --- get time for messeng --- ///
#ifdef _WIN32
            GetLocalTime(&st);
            sprintf(c, "%04d/%02d/%02d\t%02d:%02d:%02d.%03d\t", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
#endif
#ifdef __linux__
            gettimeofday(&stmsec, NULL);
            st = localtime(&stmsec.tv_sec);
            sprintf(c, "%04d/%02d/%02d\t%02d:%02d:%02d.%03d\t", st->tm_year + 1900, st->tm_mon + 1, st->tm_mday, st->tm_hour, st->tm_min, st->tm_sec, stmsec.tv_usec / 1000);
#endif

            time = c;
            helpstr = time;
            switch (current_mode)
            {
            case LogMode::DEBUG:
                helpstr += "DEBUG\t";
                break;
            case LogMode::INFO:
                helpstr += "INFO\t";
                break;
            case LogMode::WARNING:
                helpstr += "WARNING\t";
                break;
            case LogMode::ERR:
                helpstr += "ERROR\t";
                break;
            default:
                break;
            }
            helpstr += *form;
            helpstr += '\n';
        }
        catch (...) {}

        /// --- write messeng to list --- ///
        event_protect_WrDel_list.lock();
        try { LogList[work_list].push_back(helpstr); }
        catch (...) {}
        event_protect_WrDel_list.unlock();

        /// --- chek work thread --- ///
        if (mutex_turn_log.try_lock() == true)
        {
            if (AskStatusLog == Status::ON && StatusLog == Status::OFF && initthread != 1)
            {
#ifdef _WIN32 
                Sleep(1);
#endif
#ifdef __linux__ 
                usleep(1000);
#endif
                if (AskStatusLog == Status::ON && StatusLog == Status::OFF && initthread != 1) RiverWrite = std::thread(&Logger::ThreadWriteLog, this);
            }
            mutex_turn_log.unlock();
        }
    }

    void Logger::WriteLogSysMesseng(LoggerSpace::LogMode current_mode, const char** form)
    {
        MessengSysLog msng;
        std::string helpstr;
        /// --- make messeng for system log --- ///
#ifdef _WIN32
        if (current_mode == LogMode::DEBUG) msng.status = EVENTLOG_SUCCESS;
        if (current_mode == LogMode::INFO) msng.status = EVENTLOG_INFORMATION_TYPE;
        if (current_mode == LogMode::WARNING) msng.status = EVENTLOG_WARNING_TYPE;
        if (current_mode == LogMode::ERR) msng.status = EVENTLOG_ERROR_TYPE;
#endif
#ifdef __linux__
        if (current_mode == LogMode::DEBUG) msng.status = LOG_DEBUG;
        if (current_mode == LogMode::INFO) msng.status = LOG_INFO;
        if (current_mode == LogMode::WARNING) msng.status = LOG_WARNING;
        if (current_mode == LogMode::ERR) msng.status = LOG_ERR;

        switch (current_mode)
        {
        case LogMode::DEBUG:
            helpstr += "<DEBUG> ";
            break;
        case LogMode::INFO:
            helpstr += "<INFO> ";
            break;
        case LogMode::WARNING:
            helpstr += "<WARNING> ";
            break;
        case LogMode::ERR:
            helpstr += "<ERROR> ";
            break;
        default:
            break;
        }
#endif

        helpstr += *form;
        msng.messeng = helpstr;

        /// --- write messeng in list --- //
        event_protect_WrDel_Syslist.lock();
        try { LogSysList[work_syslist].push_back(msng); }
        catch (...) {}
        event_protect_WrDel_Syslist.unlock();


        /// --- chek work thread --- ///
        if (mutex_turn_syslog.try_lock() == true)
        {
            if (AskStatusSysLog == Status::ON && StatusSysLog == Status::OFF && initsysthread != 1)
            {
#ifdef _WIN32 
                Sleep(1);
#endif
#ifdef __linux__ 
                usleep(1000);
#endif
                if (AskStatusSysLog == Status::ON && StatusSysLog == Status::OFF && initsysthread != 1) RiverSysWrite = std::thread(&Logger::ThreadSysWriteLog, this);
            }
            mutex_turn_syslog.unlock();
        }

    }

    /// --- write messeng (only when work threads log) --- ///
    void Logger::WriteLogDEBUG(const char* form)
    {
        if (ModeLog == LogMode::DEBUG && AskStatusLog == Status::ON) Logger::WriteLogMesseng(LogMode::DEBUG, &form);
        if (ModeSysLog == LogMode::DEBUG && AskStatusSysLog == Status::ON) Logger::WriteLogSysMesseng(LogMode::DEBUG, &form);
    }

    void Logger::WriteLogINFO(const char* form)
    {
        if (ModeLog <= LogMode::INFO && AskStatusLog == Status::ON) Logger::WriteLogMesseng(LogMode::INFO, &form);
        if (ModeSysLog <= LogMode::INFO && AskStatusSysLog == Status::ON) Logger::WriteLogSysMesseng(LogMode::INFO, &form);
    }

    void Logger::WriteLogWARNING(const char* form)
    {

        if (ModeLog <= LogMode::WARNING && AskStatusLog == Status::ON) Logger::WriteLogMesseng(LogMode::WARNING, &form);
        if (ModeSysLog <= LogMode::WARNING && AskStatusSysLog == Status::ON) Logger::WriteLogSysMesseng(LogMode::WARNING, &form);
    }

    void Logger::WriteLogERR(const char* form)
    {
        if (AskStatusLog == Status::ON) Logger::WriteLogMesseng(LogMode::ERR, &form);
        if (AskStatusSysLog == Status::ON) Logger::WriteLogSysMesseng(LogMode::ERR, &form);
    }

    void Logger::WriteLogDEBUG(const char* form, int code_error, int syscode_error)
    {
        std::string str;
        const char* sstr;
        if (code_error == NULL) { str += "NULL\t"; }
        else { str += std::to_string(code_error) + '\t'; }

        if (syscode_error == NULL) { str += "NULL\t"; }
        else { str += std::to_string(syscode_error) + '\t'; }

        str += form;
        sstr = str.c_str();

        if (ModeLog == LogMode::DEBUG && AskStatusLog == Status::ON) Logger::WriteLogMesseng(LogMode::DEBUG, &sstr);
        if (ModeSysLog == LogMode::DEBUG && AskStatusSysLog == Status::ON) Logger::WriteLogSysMesseng(LogMode::DEBUG, &sstr);
    }

    void Logger::WriteLogINFO(const char* form, int code_error, int syscode_error)
    {
        std::string str;
        const char* sstr;
        if (code_error == NULL) { str += "NULL\t"; }
        else { str += std::to_string(code_error) + '\t'; }

        if (syscode_error == NULL) { str += "NULL\t"; }
        else { str += std::to_string(syscode_error) + '\t'; }

        str += form;
        sstr = str.c_str();

        if (ModeLog <= LogMode::INFO && AskStatusLog == Status::ON) Logger::WriteLogMesseng(LogMode::INFO, &sstr);
        if (ModeSysLog <= LogMode::INFO && AskStatusSysLog == Status::ON) Logger::WriteLogSysMesseng(LogMode::INFO, &sstr);
    }

    void Logger::WriteLogWARNING(const char* form, int code_error, int syscode_error)
    {
        std::string str;
        const char* sstr;
        if (code_error == NULL) { str += "NULL\t"; }
        else { str += std::to_string(code_error) + '\t'; }

        if (syscode_error == NULL) { str += "NULL\t"; }
        else { str += std::to_string(syscode_error) + '\t'; }

        str += form;
        sstr = str.c_str();

        if (ModeLog <= LogMode::WARNING && AskStatusLog == Status::ON) Logger::WriteLogMesseng(LogMode::WARNING, &sstr);
        if (ModeSysLog <= LogMode::WARNING && AskStatusSysLog == Status::ON) Logger::WriteLogSysMesseng(LogMode::WARNING, &sstr);
    }

    void Logger::WriteLogERR(const char* form, int code_error, int syscode_error)
    {
        std::string str;
        const char* sstr;
        if (code_error == NULL) { str += "NULL\t"; }
        else { str += std::to_string(code_error) + '\t'; }

        if (syscode_error == NULL) { str += "NULL\t"; }
        else { str += std::to_string(syscode_error) + '\t'; }

        str += form;
        sstr = str.c_str();

        if (AskStatusLog == Status::ON) Logger::WriteLogMesseng(LogMode::ERR, &sstr);
        if (AskStatusSysLog == Status::ON) Logger::WriteLogSysMesseng(LogMode::ERR, &sstr);

    }



    // --- thread log --- ///
    int Logger::ThreadWriteLog()
    {
        StatusLog = Status::ON;
        initthread = 0;
        int zz = 0;

#ifdef _WIN32
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
        HANDLE file_log = NULL;
        UINT64 t1, t2;
        SYSTEMTIME t_now;
        LARGE_INTEGER SizeFileNow;
#endif

#ifdef __linux__
        int file_log;
        timeval t_now;
        long t1, t2;
        struct stat stat_file;
#endif

        /// ---  checking the number of days of continuous recording --- ///
        std::string str;
        char numlist = 0;
        char count_empty_list = 0;
        try
        {

            for (;;)
            {
                zz++;
#ifdef _WIN32
                GetLocalTime(&t_now);
                SystemTimeToFileTime(&t_last, (FILETIME*)&t1);
                SystemTimeToFileTime(&t_now, (FILETIME*)&t2);
#endif
#ifdef __linux__
                gettimeofday(&t_now, NULL);
                t1 = t_last.tv_sec;
                t2 = t_now.tv_sec;
#endif
                t1 = t1 / 1000 / 1000 / 10 / 60 / 60 / 24;
                t2 = t2 / 1000 / 1000 / 10 / 60 / 60 / 24;
                if ((t2 - t1) >= DayWrite)
                {
                    str.clear();
                    str = NameFile;
                    str.insert(NameFile.find('.', 0), "_");
                    str.insert(str.find('.', 0), std::to_string(count_file));
#ifdef _WIN32
                    MoveFileA(NameFile.c_str(), str.c_str());
                    GetLocalTime(&t_last);
#endif
#ifdef __linux__
                    rename(NameFile.c_str(), str.c_str());
                    gettimeofday(&t_last, NULL);
#endif
                    NameFile = take_log_name(NameLog);
                    count_file = 0;
                }

                /// --- replacement of a log list --- ///
                event_protect_WrDel_list.lock();
                numlist = work_list;
                work_list = (work_list + 1) & 1;
                event_protect_WrDel_list.unlock();


                /// --- write in log file --- ///
                if (!LogList[numlist].empty())
                {
                    count_empty_list = 0;
#ifdef _WIN32
                    file_log = CreateFileA(NameFile.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_FLAG_WRITE_THROUGH, NULL);
                    SetFilePointer(file_log, 0, 0, FILE_END);
#endif
#ifdef __linux__
                    file_log = open(NameFile.c_str(), O_CREAT | O_RDWR | O_APPEND, 0xffffff);
#endif 

                    while (!LogList[numlist].empty())
                    {
                        str.clear();
                        str = LogList[numlist].front();
#ifdef _WIN32
                        WriteFile(file_log, str.c_str(), str.length(), NULL, NULL);
#endif
#ifdef __linux__
                        write(file_log, str.c_str(), str.length());
#endif 
                        LogList[numlist].pop_front();
                    }

                    /// --- check size log file --- ///
#ifdef _WIN32
                    GetFileSizeEx(file_log, &SizeFileNow);
                    if (SizeFileNow.QuadPart > SizeLogFile)
                    {
                        CloseHandle(file_log);
                        str.clear();
                        str = NameFile;
                        str.insert(NameFile.find('.', 0), "_");
                        str.insert(str.find('.', 0), std::to_string(count_file));
                        count_file++;
                        MoveFileA(NameFile.c_str(), str.c_str());
                        if (NameFile != take_log_name(NameLog))
                        {
                            NameFile = take_log_name(NameLog);
                            count_file = 0;
                        }
                        GetLocalTime(&t_last);
                    }
                    else CloseHandle(file_log);
#endif
#ifdef __linux__
                    close(file_log);
                    stat(NameFile.c_str(), &stat_file);
                    if (stat_file.st_size > SizeLogFile)
                    {
                        str.clear();
                        str = NameFile;
                        str.insert(NameFile.find('.', 0), "_");
                        str.insert(str.find('.', 0), std::to_string(count_file));
                        count_file++;
                        rename(NameFile.c_str(), str.c_str());
                        if (NameFile != take_log_name(NameLog))
                        {
                            NameFile = take_log_name(NameLog);
                            count_file = 0;
                        }
                        gettimeofday(&t_last, NULL);
                    }
#endif
                }
                else
                {
                    count_empty_list++;
                    if (count_empty_list >= 2)
                    {
                        count_empty_list = 0;
                        mutex_turn_log.lock();
                        if (AskStatusLog == Status::OFF)
                        {
                            StatusLog = Status::OFF;
                            mutex_turn_log.unlock();
                            break;
                        }
                        mutex_turn_log.unlock();
#ifdef _WIN32
                        Sleep(1);
#endif 
#ifdef __linux__
                        usleep(1000);
#endif                        
                    }
                }
            }
        }
        catch (...)
        {
            StatusLog = Status::OFF;
        }
        return 0;
    }
}

