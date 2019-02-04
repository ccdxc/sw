///=================================
/// Rich Message Manager
/// 
/// Pensando, All Rights Reserved.
///
///    by changqi.yang@pensando.io
///                      6/17/2017
//

#ifndef __CPP_MSG_MAN_HH__
#define __CPP_MSG_MAN_HH__

#include <vector>
#include <set>
#include <map>
#include <list>
#include <iostream>
#include <ostream>
#include <iomanip>
#include "LogMsg.h"
#include "msg_stream.h"

using namespace std;
class msg_man;


#define MSG_MAN_HELPER_LOOP_STREAM_BEGIN \
    for (vector<msg_stream*>::iterator it = streams.begin(); it != streams.end(); it++)  { \
        msg_stream* stream = *it; \
        const msg_scope* scope = current_scope; \
        if(!scope) {} \
        msg_level_t level = current_level; \
        if(level) {} 

#define MSG_MAN_HELPER_LOOP_STREAM_END }

#define MSG_MAN_HELPER_STREAM_ACTIVE_BEGIN \
    if (stream && stream->enabled() && \
        (stream->color || \
         (stream->levels.find(scope) != stream->levels.end() && \
          stream->pass_level_test((msg_stream::msg_level_t)level, scope)))) {

#define MSG_MAN_HELPER_STREAM_ACTIVE_END }

typedef ostream& (*iostream_endl_func_t)(ostream&);
typedef ios_base& (*iostream_format_func_t)(ios_base&);

class msg_scope {
private:
    msg_scope(msg_man& m);
    ~msg_scope();
    void set_parent(msg_scope* p);
    void add_child(msg_scope* c);
    void insert_stream(msg_stream* s, bool deep);
    void remove_stream(msg_stream* s, bool deep);

private:
    msg_man&                                        man;
    msg_scope*                                      parent;
    vector<msg_scope*>                              children;
    set<msg_stream*>                                streams;
    string                                          name;

private:
    friend class msg_man;
    friend class msg_stream;
};


class msg_man {
public:
    typedef enum {
        error       = LogMsg::ERROR,
        warning     = LogMsg::WARNING,
        msg         = LogMsg::MSG,
        info        = LogMsg::INFO,
        cpu         = LogMsg::CPU,
        debug       = LogMsg::DEBUG
    } msg_level_t;

    typedef enum {
        ctl_reset = 0, 
        ctl_begin,
        ctl_save_format,
        ctl_restore_format
    } control_t;

public:
    ///////USER API
    static msg_man* instancep();
    static msg_man& instance();
    static void init();

    msg_man();
    ~msg_man();

    template<typename T> bool open(T*& s);
    bool open(msg_stream_file*& s, const string& name, const string& filename);
    void close(msg_stream* s);
    msg_stream_cout*&  stream_cout() { return s_cout; }
    msg_stream_cerr*&  stream_cerr() { return s_cerr; }
    msg_man& operator << (msg_level_t lvl) { return change_level(lvl); }
    msg_man& operator << (msg_scope& scope) { return change_scope(scope); }
    msg_man& operator << (control_t ctl) { return control_me(ctl); }
    msg_man& operator << (iostream_endl_func_t o);
    msg_man& operator << (iostream_format_func_t o);
    msg_man& operator << (const struct _Setw & s);
    template<typename T> msg_man& operator << (const struct _Setfill<T> & s);
    template<typename T> msg_man& operator << (const T& message) { write(message); return *this; }
    void set_default(msg_level_t lvl, const msg_scope* scope);
    msg_scope* create_or_get_scope(const string& name);
    const string& get_current_scope_name() const { return current_scope->name; }
    static void display_formatted_data(const list<int>& data, bool hex = true, int max_chars = 2, char fill = '0', bool insert_0x = false, int sec_num = 8, int row_num = 32, int indent = 0, const char* element_delimiter = " ", const char* sec_delimiter = "    ", const char* line_delimiter = 0);
    static void display_formatted_data(const unsigned char* data, int len, bool hex = true, int max_chars = 2, char fill = '0', bool insert_0x = false, int sec_num = 8, int row_num = 32, int indent = 0, const char* element_delimiter = " ", const char* sec_delimiter = "    ", const char* line_delimiter = 0);
    static void diff_formatted_data(const unsigned char* data0, const unsigned char* data1, int len, bool hex = true, int max_chars = 2, char fill = '0', bool insert_0x = false, int sec_num = 8, int row_num = 32, int indent = 0, const char* element_delimiter = " ", const char* sec_delimiter = "    ", const char* line_delimiter = 0);



private:
    typedef void (msg_man::*stream_proc_func_t)(msg_stream* stream, const msg_scope* scope, msg_level_t level, const string& str);

    static string display_level(msg_level_t lvl);
    void reset();
    msg_man& change_level(msg_level_t lvl);
    msg_man& change_scope(msg_scope& scope);
    msg_man& control_me(control_t ctl);
    void parse_scope_names(const string& name, vector<string>& name_hier);
    msg_scope* create_or_get_scope(msg_scope* parent, vector<string>::iterator& name_it);
    void travel_streams(stream_proc_func_t f, const string& message);
    void begin_message(msg_stream* stream, const msg_scope* scope, msg_level_t level, const string& str);
    template<typename T> void write(const T& message);
    template<typename T> static void print_1_data(const T& data, bool hex, int max_chars, char fill, bool insert_ox, const char* pfx = NULL, const char* sfx = NULL);

private:
    bool                                            init_done;
    msg_stream_cout*                                s_cout;
    msg_stream_cerr*                                s_cerr;
    msg_level_t                                     current_level;
    const msg_scope*                                current_scope;
    msg_level_t                                     default_level;
    const msg_scope*                                default_scope;
    msg_scope*                                      scope_root;
    map<string, msg_scope*>                         map_name2scope;
    vector<msg_stream*>                             streams;
    map<string, msg_stream*>                        map_name2stream;

private:
    friend class msg_stream;
};


template<typename T>
bool msg_man::open(T*& s)
{
    if (s) {
        close(s);
        s->open();
        return s->enabled();
    }
    s = new T;
    if (s) {
        s->open();
        map_name2stream[s->name] = s;
        streams.push_back(s);
        return s->enabled();
    }
    else {
        return false;
    }
}


template<typename T>
void msg_man::write(const T& message)
{
    MSG_MAN_HELPER_LOOP_STREAM_BEGIN
    MSG_MAN_HELPER_STREAM_ACTIVE_BEGIN
        (stream->convert_to_ostream()) << message;
        stream->tag_inserted = false;
    MSG_MAN_HELPER_STREAM_ACTIVE_END
    MSG_MAN_HELPER_LOOP_STREAM_END
}


template<typename T>
msg_man& msg_man::operator << (const struct _Setfill<T> & s)
{
    MSG_MAN_HELPER_LOOP_STREAM_BEGIN
    MSG_MAN_HELPER_STREAM_ACTIVE_BEGIN
        (stream->convert_to_ostream()) << s;
    MSG_MAN_HELPER_STREAM_ACTIVE_END
    MSG_MAN_HELPER_LOOP_STREAM_END

    return *this;
}


string itos(uint64_t v);


#define MSG_MAN_P  (msg_man::instancep())
#define MSG_MAN    (msg_man::instance())

/////////////////////// USER MACROS
#define MSG_STREAMP_COUT (MSG_MAN_P->stream_cout())
#define MSG_STREAMP_CERR (MSG_MAN_P->stream_cerr())
#define MSG_STREAM_COUT (*MSG_STREAMP_COUT)
#define MSG_STREAM_CERR (*MSG_STREAMP_CERR)

#define MSGO (MSG_MAN << msg_man::ctl_begin)
#define MSGX (MSG_MAN)

#define MSG_ERROR (MSG_MAN << msg_man::error << msg_man::ctl_begin)
#define MSG_WARNING (MSG_MAN << msg_man::warning << msg_man::ctl_begin)
#define MSG_MSG (MSG_MAN << msg_man::msg << msg_man::ctl_begin)
#define MSG_INFO (MSG_MAN << msg_man::info << msg_man::ctl_begin)
#define MSG_CPU (MSG_MAN << msg_man::cpu << msg_man::ctl_begin)
#define MSG_DEBUG (MSG_MAN << msg_man::debug << msg_man::ctl_begin)

#define MSG_ERROR_SCOPE(X) (MSG_MAN << (*(MSG_MAN_P->create_or_get_scope(X))) << msg_man::error << msg_man::ctl_begin)
#define MSG_WARNING_SCOPE(X) (MSG_MAN << (*(MSG_MAN_P->create_or_get_scope(X))) << msg_man::warning << msg_man::ctl_begin)
#define MSG_MSG_SCOPE(X) (MSG_MAN << (*(MSG_MAN_P->create_or_get_scope(X))) << msg_man::msg << msg_man::ctl_begin)
#define MSG_INFO_SCOPE(X) (MSG_MAN << (*(MSG_MAN_P->create_or_get_scope(X))) << msg_man::info << msg_man::ctl_begin)
#define MSG_CPU_SCOPE(X) (MSG_MAN << (*(MSG_MAN_P->create_or_get_scope(X))) << msg_man::cpu << msg_man::ctl_begin)
#define MSG_DEBUG_SCOPE(X) (MSG_MAN << (*(MSG_MAN_P->create_or_get_scope(X))) << msg_man::debug << msg_man::ctl_begin)

#define MSG_SET_SCOPE(X) (MSG_MAN << (*(MSG_MAN_P->create_or_get_scope(X))))

#define COUT_ERROR_SCOPE_BEGIN(X) \
    if (MSG_STREAMP_COUT->pass_level_test(msg_stream::error, MSG_MAN_P->create_or_get_scope(X)))
#define COUT_WARNING_SCOPE_BEGIN(X) \
    if (MSG_STREAMP_COUT->pass_level_test(msg_stream::warning, MSG_MAN_P->create_or_get_scope(X)))
#define COUT_MSG_SCOPE_BEGIN(X) \
    if (MSG_STREAMP_COUT->pass_level_test(msg_stream::msg, MSG_MAN_P->create_or_get_scope(X)))
#define COUT_INFO_SCOPE_BEGIN(X) \
    if (MSG_STREAMP_COUT->pass_level_test(msg_stream::info, MSG_MAN_P->create_or_get_scope(X)))
#define COUT_DEBUG_SCOPE_BEGIN(X) \
    MSG_DEBUG_SCOPE(X) << "BEGIN >>>" << endl; \
    if (MSG_STREAMP_COUT->pass_level_test(msg_stream::debug, MSG_MAN_P->create_or_get_scope(X)))



template<typename T>
void msg_man::print_1_data(const T& data, bool hex, int max_chars, char fill, bool insert_0x, const char* pfx, const char* sfx)
{
    if (pfx) { MSGX << pfx; }
    if (hex && insert_0x) { MSGX << "0x"; }
    if (max_chars > 0 || fill != 0) { MSGX << msg_man::ctl_save_format; }
    if (hex) { MSGX << std::hex; }
    if (max_chars > 0) { MSGX << setw(max_chars); }
    if (fill != 0) { MSGX << setfill(fill); }
    MSGX << data;
    if (hex) { MSGX << std::dec; }
    if (max_chars > 0 || fill != 0) { MSGX << msg_man::ctl_restore_format; }
    if (sfx) { MSGX << sfx; }
}



//////macro for sknobs warnings
#define SKNOBS_GET(x, d) \
    sknobs_get_value(const_cast<char*>(x), (d))

#define SKNOBS_GETD(x, d) \
    sknobs_get_dynamic_value(const_cast<char*>(x), (d))

#define SKNOBS_GETS(x, d) \
    sknobs_get_string(const_cast<char*>(x), const_cast<char*>(d))

#define SKNOBS_SET(x, val) \
    sknobs_set_value(const_cast<char*>(x), (val))

#define SKNOBS_SETS(x, val) \
    sknobs_set_string(const_cast<char*>(x), const_cast<char*>(val))


//
#define PKNOB_HELPER_GET(V, T, X, D) \
    if (PKnobsReader::knobExists((X))) { \
        (V) = (T)PKnobsReader::evalKnob((X)); \
    } else { \
        (V) = (T)SKNOBS_GETD(((X)), (D)); \
    }


#define PKNOB_HELPER_GETS(V, T, X, D) \
    if (PKnobsReader::knobExists((X))) { \
        (V) = (T)PKnobsReader::evalKnob((X)); \
    } else { \
        (V) = (T)SKNOBS_GETD(((X)).c_str(), (D)); \
    }




#endif //__CPP_MSG_MAN_HH__
