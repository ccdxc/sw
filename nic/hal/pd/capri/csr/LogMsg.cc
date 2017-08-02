#include "LogMsg.h"

#ifdef _CSV_INCLUDED_
#include "vpi_user.h"
#endif

LogMsg::LogMsg() {
    //Read sknobs to set plog Level
    if (sknobs_exists((char *)"plog")) {
        string plogLevel = sknobs_get_string((char *)"plog", (char *)"");
        if(plogLevel == "err") {
           setVerbose(ERROR);
        } else if(plogLevel == "warn") {
           setVerbose(WARNING);
        } else if(plogLevel == "info") {
           setVerbose(INFO);
        } else if(plogLevel == "cpu") {
           setVerbose(CPU);
        } else if(plogLevel == "debug") {
           setVerbose(DEBUG);
        } else {
           setVerbose(MSG);
        }
    } else {
        setVerbose(MSG);
    }

    //Read sknobs to add plog scope
    if (sknobs_exists((char *)"plogAddScope")) {
        string plogScope = sknobs_get_string((char *)"plogAddScope", (char *)"");
        enableScope(plogScope);
    }
    if (sknobs_exists((char *)"plog_add_scope")) {
        string plogScope = sknobs_get_string((char *)"plog_add_scope", (char *)"");
        enableScope(plogScope);
    }

    //Read sknobs to remove plog scope
    if (sknobs_exists((char *)"plogRmScope")) {
        string plogScope = sknobs_get_string((char *)"plogRmScope", (char *)"");
        disableScope(plogScope);
    }
    if (sknobs_exists((char *)"plog_rm_scope")) {
        string plogScope = sknobs_get_string((char *)"plog_rm_scope", (char *)"");
        disableScope(plogScope);
    }
}

void LogMsg::enableScope(string scope) {
    enabledScopes.push_back(scope);
}

//stringstream & LogMsg::getLocalStringStream() {
//    return localStringStream;
//}
void LogMsg::setVerbose(msgLevelEnum level) {
    curMsgLevel = static_cast<unsigned int>(level);
}
LogMsg::msgLevelEnum LogMsg::getVerbose(void) {
    return static_cast<msgLevelEnum>(curMsgLevel);
}
void LogMsg::disableScope(string scope) {
    enabledScopes.erase(std::remove(enabledScopes.begin(), enabledScopes.end(), scope), enabledScopes.end());
}
void LogMsg::printMsg(msgLevelEnum level, string msg) {
    if(checkMsgLevel(level)) {
        string level_str = convertLevelToStr(level);
        logNow("" + level_str + ": " + msg);
    }
}
void LogMsg::printMsg(string scope, string msg) {
    if(checkMsgLevel(scope)) {
        logNow("" + scope + ": " + msg);
    }
}
bool LogMsg::checkMsgLevel(msgLevelEnum level) {
    if( static_cast<unsigned int>(level) <= curMsgLevel) {
        return true; 
    }
    return false;
}
bool LogMsg::checkMsgLevel(string scope) {
    vector<string>::iterator p = std::find (enabledScopes.begin(), enabledScopes.end(),  scope);
    if(p != enabledScopes.end()) {
        return true;
    }
    return false;
}

string LogMsg::convertLevelToStr(msgLevelEnum level) {
    if(level == ERROR) {
        return "ERROR";
    } else if(level == WARNING) {
        return "WARNING";
    } else if(level == MSG) {
        return "MSG";
    } else if(level == INFO) {
        return "INFO";
    } else if(level == CPU) {
        return "CPU";
    } else if(level == DEBUG) {
        return "DEBUG";
    } else {
        return "UNKNOWN";
    }
}

void LogMsg::logNow(string msg) {
#ifdef _CSV_INCLUDED_
    //s_vpi_time time;
    //time.type = vpiSimTime;
    //vpi_get_time(NULL, &time);
    //vpi_printf("C++ %d%d %s",time.high, time.low, msg);
    vpi_printf((PLI_BYTE8*) "C++ :: %s", msg.c_str());
#else 
    std::cout << msg;
#endif    
}
