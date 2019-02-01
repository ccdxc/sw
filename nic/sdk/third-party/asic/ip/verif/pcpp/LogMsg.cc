#include "LogMsg.h"
#include "common_dpi.h"

LogMsg::LogMsg() {

    m_cur_error_count = 0;

    m_max_fsize = 1<<24; //16MB
    m_enb_flog = true;
    m_fptr = NULL;
    m_fname = "_undefined_";

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

    // +PLOG_MAX_QUIT_COUNT=0 will disable this
    m_max_error_count = sknobs_get_value((char *)"PLOG_MAX_QUIT_COUNT", 10);

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

////////////////////////////////////////////////////////////////////
void LogMsg::closeFlog( ) {

  if(m_fptr) {
  if(m_fptr->is_open()) {
    printf("\n>>>>>>Closing file : %s\n\n", m_fname.c_str());
    m_fptr->close();
    delete m_fptr;
    m_fptr = NULL;
    m_fname = "_undefined";
  }
  }
}

////////////////////////////////////////////////////////////////////
void LogMsg::startFlog( const string & fName, const uint32_t size) {

  if (!m_enb_flog) {
    printf("File logging is disabled, ignoring startFlog\n");
    return;
  }
 
  if (m_fptr !=  NULL) { closeFlog(); }

  m_fptr = new std::ofstream;
  m_fptr->open(fName.c_str(), ios::out | ios::trunc);
  m_fname = fName;
  m_max_fsize = size;
  printf("\n>>>>>>File logging started, filename : %s , file size limited to:%d bytes\n\n", fName.c_str(), size);
}

////////////////////////////////////////////////////////////////////
void LogMsg::stopFlog() {

  if (!m_enb_flog) {
    printf("LogMsg::stopFlog : File logging is disabled, ignoring stopFlog\n");
    return;
  }
 
  if (m_fptr !=  NULL) {
    closeFlog();
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
    std::remove(enabledScopes.begin(), enabledScopes.end(), scope);
}
void LogMsg::printMsg(msgLevelEnum level, string msg) {
  if(checkMsgLevel(level)) {
    string level_str = convertLevelToStr(level);
    /*
    char simt[128];
#ifdef _CSV_INCLUDED_
    dpi_sv_get_sim_time_ns(simt);
#else
    simt[0] = 0;
#endif
    logNow("" + level_str + " :" + simt + ": " + msg);
    */
    logNow("" + level_str + " :: " + msg);

    if (level == ERROR) {
      m_cur_error_count++;
      if (m_max_error_count > 0) {
        if (m_cur_error_count >= m_max_error_count) {
          string s = "PLOG: max_err_count reached m_cur_error_count="+ std::to_string(m_cur_error_count) +" m_max_error_count="+std::to_string(m_max_error_count)+" , exiting simulation\n" ;
          logNow(s);
#ifndef _COSIM_
          exitSim();
#endif
        }
      }
    }


  }
}
void LogMsg::printMsg(string scope, string msg) {
    if(checkMsgLevel(scope)) {
/*
        char simt[128];
#ifdef _CSV_INCLUDED_
        dpi_sv_get_sim_time_ns(simt);
#else
        simt[0] = 0;
#endif
        logNow("" + scope + " :" + simt + ": " + msg);
*/
        logNow("" + scope + " :: " + msg);
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
    s_vpi_time time;
    time.type = vpiSimTime;
    vpi_get_time(NULL, &time);
    vpi_printf((PLI_BYTE8*) "C++ @ %lu %s",   ((unsigned long long)time.high << 32)| time.low, msg.c_str());
#else 
    std::cout << msg;

    //log to file 
    if (m_enb_flog) {
      if (m_fptr !=  NULL) {
        *m_fptr <<  msg;
        if( m_fptr->tellp() > m_max_fsize) {
          printf("File Logging :  Max Size %d bytes reached, Closing file %s\n", m_max_fsize, m_fname.c_str());
          stopFlog();
        }
      }
    }

#endif    
}

void LogMsg::exitSim() {
#ifdef _CSV_INCLUDED_
    vpi_control(vpiFinish,0);
#else 
    exit(0);
#endif    
}

