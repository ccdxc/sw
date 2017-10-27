#ifndef _LOG_MSG_H_
#define _LOG_MSG_H_

#include <boost/shared_ptr.hpp>

#include <iostream>
#include <sstream>
#include <vector>
#include "string.h"
#include "sknobs.h"
#ifdef _CSV_INCLUDED_
#include "vpi_user.h"
#endif

using namespace std;
class LogMsg {

    private:
        uint32_t m_cur_error_count;
        uint32_t m_max_error_count;

    public:
        enum msgLevelEnum { ERROR = 1, WARNING = 2, MSG = 10, INFO = 20, CPU = 30, DEBUG = 40}; 
    protected:
        LogMsg();
        vector<string> enabledScopes;
        unsigned int curMsgLevel;
        void logNow(string msg);
        void exitSim();
        //stringstream localStringStream;

    public:
        virtual ~LogMsg() {}

        virtual void enableScope(string scope);
        virtual void setVerbose(msgLevelEnum level);
        virtual msgLevelEnum getVerbose(void);
        virtual void disableScope(string scope);
        virtual void printMsg(msgLevelEnum level, string msg);
        virtual void printMsg(string scope, string msg);
        virtual bool checkMsgLevel(msgLevelEnum level);
        virtual bool checkMsgLevel(string scope);
        virtual string convertLevelToStr(msgLevelEnum level);
        //virtual stringstream & getLocalStringStream();
        static std::shared_ptr<LogMsg> Instance()
        {
            static std::shared_ptr<LogMsg> s{new LogMsg};
            return s;
        }

};

#define PLOG_MSG_GLB(X, LEVEL) {\
        if(LogMsg::Instance().get()->checkMsgLevel(LEVEL)) { \
             { \
            stringstream l_ss;\
            l_ss << X; \
            LogMsg::Instance().get()->printMsg(LEVEL, l_ss.str()); \
            } \
        }\
}

#define PLOG_MSG(X)  { PLOG_MSG_GLB(X, LogMsg::MSG) }
#define PLOG_INFO(X)  { PLOG_MSG_GLB(X, LogMsg::INFO) }
#define PLOG_ERR(X)  { PLOG_MSG_GLB(X, LogMsg::ERROR) }
#define PLOG_WARN(X)  { PLOG_MSG_GLB(X, LogMsg::WARNING) }
#define PLOG_CPU(X)  { PLOG_MSG_GLB(X, LogMsg::CPU) }
#define PLOG_DEBUG(X)  { PLOG_MSG_GLB(X, LogMsg::DEBUG) }

#define PLOG(SCOPE,X) { PLOG_MSG_GLB(X, SCOPE) }

#define PLOG_API_MSG(SRC, MSG) { PLOG_MSG(SRC << "_API : " << MSG)  }

#define PLOG_CHECK_MSG_LEVEL(LEVEL) LogMsg::Instance().get()->checkMsgLevel(LEVEL)

#ifdef _CSV_INCLUDED_
#define PEN_ASSERT(c) if (!(c)) {PLOG_MSG("Assertion failed: " << __FILE__ << ":" << __LINE__ << " function: " << __PRETTY_FUNCTION__ << " " << #c << endl); vpi_control(vpiFinish,0);}
#else
#define PEN_ASSERT(c) assert(c)
#endif

#endif // _LOG_MSG_H_
