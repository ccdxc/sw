///=================================
/// Rich Message Stream 
/// 
/// Pensando, All Rights Reserved.
///
///    by changqi.yang@pensando.io
///                      6/17/2017

#ifndef __CPP_MSG_STREAM_HH__
#define __CPP_MSG_STREAM_HH__

#include <string>
#include <fstream>
#include <set>
#include <map>
#include "LogMsg.h"

class msg_scope;
class msg_man;

class msg_stream {
public:
    typedef enum {
        error       = LogMsg::ERROR,
        warning     = LogMsg::WARNING,
        msg         = LogMsg::MSG,
        info        = LogMsg::INFO,
        cpu         = LogMsg::CPU,
        debug       = LogMsg::DEBUG
    } msg_level_t;


public: //////user API
    void set_scope(const string& scope_name, bool deep = false);        ///remove existing scopes and set to the new one
    void add_scope(const string& scope_name, bool deep = false);        ///add new scopes
    void remove_scope(const string& scope_name, bool deep = false);     ///remove from all existing scopes
    void set_level(const string& scope_name, msg_level_t level, bool deep = false); ////set level to corresponding scope
    void set_high_level(const string& scope_name, msg_level_t level, bool deep = false);  ///high level is used for "lvl >= high_level && lvl <= level" test
    void set_insert_level(bool yes = true) { insert_level = yes; }
    void set_insert_scope(bool yes = true) { insert_scope = yes; }
    void set_insert_name(bool yes = true) { insert_name = yes; }
    void set_insert_time(bool yes = true) { insert_time = yes; }
    void set_deep_scope(bool yes = true) { deep_scope = yes; update_scope(); } 
    void set_enable(bool yes = true) { enable = yes; }
    bool enabled() const { return enable; }
    virtual bool pass_level_test(msg_level_t lvl, const msg_scope* scope);
    void set_default_level(msg_level_t lvl) { default_level = lvl; }
    void set_default_high_level(msg_level_t lvl) { default_high_level = lvl; }


protected: //////customization API
    virtual ostream& convert_to_ostream() = 0;
    virtual void open();
    virtual void close() { enable = false; }


protected:
    msg_stream(msg_man& man, const string& name);
    virtual ~msg_stream();

private:
    void begin_msg(const msg_scope* scope, msg_level_t level);
    void update_scope();
    void set_level_recursive(msg_scope* scope, msg_level_t level);
    void set_high_level_recursive(msg_scope* scope, msg_level_t level);
    void save_format();
    void restore_format();
    
private:
    msg_man&                            man;
    set<msg_scope*>                     scopes;
    map<const msg_scope*, msg_level_t>  levels;   
    map<const msg_scope*, msg_level_t>  high_levels;   
    msg_level_t                         default_level;
    msg_level_t                         default_high_level;
    bool                                insert_name;
    bool                                insert_time;
    bool                                insert_level;
    bool                                insert_scope;
    string                              name;
    bool                                deep_scope;
    bool                                enable;
    bool                                tag_inserted;
    ios_base::fmtflags                  format;
    int                                 color;

private:
    friend class msg_man;
    friend class msg_scope;
};

class msg_stream_cerr;
class msg_stream_cout: public msg_stream
{
protected:
    virtual void open();
    virtual ostream& convert_to_ostream();

private:
    msg_stream_cout();

private:
    friend class msg_stream_cerr;
    friend class msg_man;
};


class msg_stream_cerr: public msg_stream
{
protected:
    virtual ostream& convert_to_ostream();
    virtual void open();
    virtual void close();
private:
    msg_stream_cerr();
private:
    friend class msg_man;
};


class msg_stream_file: public msg_stream
{
protected:
    virtual ostream& convert_to_ostream();
    virtual void open();
    virtual void close();
private:
    msg_stream_file(const string& name, const string& filename);
private:
    string                              file_name;
    ofstream                            file_stream;
private:
    friend class msg_man;
};


#endif //__CPP_MSG_STREAM_HH__
