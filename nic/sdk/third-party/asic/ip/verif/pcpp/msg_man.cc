///=================================
/// Rich Message Manager
/// 
/// Pensando, All Rights Reserved.
///
///    by changqi.yang@pensando.io
///                      6/17/2017
//

#include <memory>
#include <iostream>
#include "msg_man.h"





msg_scope::msg_scope(msg_man& m) : man(m)
{
    parent = 0;
}


msg_scope::~msg_scope()
{
}


void msg_scope::set_parent(msg_scope* p)
{
    parent = p;
}


void msg_scope::add_child(msg_scope* c)
{
    children.push_back(c);
    c->set_parent(this);

    for (set<msg_stream*>::iterator it = streams.begin(); it != streams.end(); it++) {
        if ((*it)->deep_scope) c->insert_stream(*it, false);
    }
}


void msg_scope::insert_stream(msg_stream* s, bool deep)
{
    streams.insert(s);
    s->scopes.insert(this);
    if (parent && s->levels.find(parent) != s->levels.end()) {
        s->levels[this] = s->levels[parent];
    }
    else {
        s->levels[this] = s->default_level;
    }

    if (deep) {
        for (vector<msg_scope*>::iterator it = children.begin(); it != children.end(); ++it) {
            (*it)->insert_stream(s, deep);
        }
    }
}


void msg_scope::remove_stream(msg_stream* s, bool deep)
{
    streams.erase(s);
    s->scopes.erase(this);
    s->levels.erase(this);
    if (deep) {
        for (vector<msg_scope*>::iterator it = children.begin(); it != children.end(); ++it) {
            (*it)->remove_stream(s, deep);
        }
    }
}


msg_man* msg_man::instancep()
{
    static std::unique_ptr<msg_man> g_msg_man = std::unique_ptr<msg_man>(new msg_man);
    return g_msg_man.get();
}


msg_man& msg_man::instance()
{
    return *(instancep());
}


msg_man::msg_man()
{
    init_done = false;
    s_cout = 0;
    s_cerr = 0;
    scope_root = create_or_get_scope("/");
    set_default(msg, scope_root);
    reset();
}


msg_man::~msg_man()
{
    for (map<string, msg_scope*>::iterator it = map_name2scope.begin(); it != map_name2scope.end(); it++) {
        delete it->second;
    }
    for (vector<msg_stream*>::iterator it = streams.begin(); it != streams.end(); it++) {
        delete (*it);
    }
}


void msg_man::set_default(msg_level_t lvl, const msg_scope* scope)
{
    default_level = lvl;
    default_scope = scope;
}


void msg_man::reset()
{
    current_level = default_level;
    current_scope = default_scope;
}


msg_man& msg_man::change_level(msg_level_t lvl)
{
    if (current_level != lvl) {
        MSG_MAN_HELPER_LOOP_STREAM_BEGIN
            stream->color = 0;
        MSG_MAN_HELPER_LOOP_STREAM_END
    }
    current_level = lvl;
    return *this;
}


msg_man& msg_man::change_scope(msg_scope& scope)
{
    if (current_scope != &scope) {
        MSG_MAN_HELPER_LOOP_STREAM_BEGIN
            stream->color = 0;
        MSG_MAN_HELPER_LOOP_STREAM_END
    }

    current_scope = &scope;
    return *this;
}


msg_man& msg_man::control_me(control_t ctl)
{
    switch (ctl) {
        case ctl_reset: 
        {
            reset();
            break;
        }
        case ctl_begin:
        {
            travel_streams(&msg_man::begin_message, "");
            break;
        }
        case ctl_save_format:
        {
            MSG_MAN_HELPER_LOOP_STREAM_BEGIN
            MSG_MAN_HELPER_STREAM_ACTIVE_BEGIN
                stream->save_format();
            MSG_MAN_HELPER_STREAM_ACTIVE_END
            MSG_MAN_HELPER_LOOP_STREAM_END
            break;
        }
        case ctl_restore_format:
        {
            MSG_MAN_HELPER_LOOP_STREAM_BEGIN
            MSG_MAN_HELPER_STREAM_ACTIVE_BEGIN
                stream->restore_format();
            MSG_MAN_HELPER_STREAM_ACTIVE_END
            MSG_MAN_HELPER_LOOP_STREAM_END
            break;
        }
    }

    return *this;
}


void msg_man::parse_scope_names(const string& name, vector<string>& name_hier)
{
    name_hier.clear();
    
    if (name.empty()) {
        cerr << "[FATAL] illegal scope name: empty string." << endl;
        exit(1000);
    }
    if (name[0] != '/') {
        cerr << "[FATAL] illegal scope name '" << name << "': must start with '/'" << endl;
        exit(1000);
    }
    if (name.length() > 1 && name[name.length() - 1] == '/') {
        cerr << "[FATAL] illegal scope name '" << name << "': must not end with '/'" << endl;
        exit(1000);
    }


    name_hier.push_back("/");

    string currnt = "/";
    string remain = name.substr(1, string::npos);
    while (!remain.empty()) {
        if (remain[0] == '/') {
            cerr << "[FATAL] illegal scope name '" << name << "': more than 1 '/' is not allowed as delimiter of hierarchy." << endl;
            exit(1000);
        }
        size_t p = remain.find('/');
        string nextname;
        if (p == string::npos) {
            nextname = remain;
            remain = "";
        }
        else {
            nextname = remain.substr(0, p);
            remain = remain.substr(p + 1, string::npos);
        }
        currnt += nextname;
        name_hier.push_back(currnt);
        currnt += "/";
    }
}



msg_scope* msg_man::create_or_get_scope(msg_scope* parent, vector<string>::iterator& name_it)
{
    msg_scope* scope = 0;
    map<string, msg_scope*>::iterator it = map_name2scope.find(*name_it);
    if (it == map_name2scope.end()) {
        scope = new msg_scope(*this);
        if (parent) {
            parent->add_child(scope);
        }
        else scope->set_parent(0);
        map_name2scope[*name_it] = scope;
        scope->name = *name_it;
    }
    else {
        scope = it->second;
    }
    name_it++;
    return scope;
}


msg_scope* msg_man::create_or_get_scope(const string& name)
{
    vector<string> namevec;
    parse_scope_names(name, namevec);

    msg_scope* p = 0;
    vector<string>::iterator it = namevec.begin();
    while (it != namevec.end()) {
        p = create_or_get_scope(p, it);
    }

    return p;
}



void msg_man::init()
{
    if (MSG_MAN_P->init_done) return;
    MSG_MAN_P->open(MSG_STREAMP_COUT);
    MSG_STREAM_COUT.set_default_level(msg_stream::msg);
    MSG_STREAM_COUT.set_default_high_level(msg_stream::error);

    string msg_scope = SKNOBS_GETS("msg_man/scope", "");
    if (msg_scope == "") msg_scope = "/";
    string msg_level = SKNOBS_GETS("msg_man/level", "");
    if (msg_level == "error") {
        if (msg_scope == "/") MSG_STREAM_COUT.set_default_level(msg_stream::error);
        MSG_STREAM_COUT.set_level(msg_scope, msg_stream::error, true);
    }
    else if (msg_level == "warning") {
        if (msg_scope == "/") MSG_STREAM_COUT.set_default_level(msg_stream::warning);
        MSG_STREAM_COUT.set_level(msg_scope, msg_stream::warning, true);
    }
    else if (msg_level == "msg") {
        if (msg_scope == "/") MSG_STREAM_COUT.set_default_level(msg_stream::msg);
        MSG_STREAM_COUT.set_level(msg_scope, msg_stream::msg, true);
    }
    else if (msg_level == "info") {
        if (msg_scope == "/") MSG_STREAM_COUT.set_default_level(msg_stream::info);
        MSG_STREAM_COUT.set_level(msg_scope, msg_stream::info, true);
    }
    else if (msg_level == "debug") {
        if (msg_scope == "/") MSG_STREAM_COUT.set_default_level(msg_stream::debug);
        MSG_STREAM_COUT.set_level(msg_scope, msg_stream::debug, true);
    }

    MSG_MAN_P->init_done = true;
}


void msg_man::close(msg_stream* s)
{
    scope_root->remove_stream(s, true);
    s->close();
}


void msg_man::travel_streams(stream_proc_func_t f, const string& str)
{
    MSG_MAN_HELPER_LOOP_STREAM_BEGIN        
        (this->*f)(stream, scope, level, str);
    MSG_MAN_HELPER_LOOP_STREAM_END
}


void msg_man::begin_message(msg_stream* stream, const msg_scope* scope, msg_level_t level, const string& str)
{
    MSG_MAN_HELPER_STREAM_ACTIVE_BEGIN
        stream->begin_msg(scope, (msg_stream::msg_level_t)level);
    MSG_MAN_HELPER_STREAM_ACTIVE_END
}


string msg_man::display_level(msg_level_t lvl)
{
    switch (lvl) {
        case error: return "ERROR";
        case warning: return "WARNING";
        case msg: return "MSG";
        case info: return "INFO";
        case cpu: return "CPU";
        case debug: return "DEBUG";
        default: return "UNKNOWN";
    }
}


msg_man& msg_man::operator << (iostream_endl_func_t o)
{
    MSG_MAN_HELPER_LOOP_STREAM_BEGIN
    MSG_MAN_HELPER_STREAM_ACTIVE_BEGIN
        (stream->convert_to_ostream()) << o;
        stream->tag_inserted = false;
    MSG_MAN_HELPER_STREAM_ACTIVE_END
    MSG_MAN_HELPER_LOOP_STREAM_END

    return *this;
}



msg_man& msg_man::operator << (const struct _Setw & s)
{
    MSG_MAN_HELPER_LOOP_STREAM_BEGIN
    MSG_MAN_HELPER_STREAM_ACTIVE_BEGIN
        (stream->convert_to_ostream()) << s;
    MSG_MAN_HELPER_STREAM_ACTIVE_END
    MSG_MAN_HELPER_LOOP_STREAM_END

    return *this;
}



msg_man& msg_man::operator << (iostream_format_func_t o)
{
    MSG_MAN_HELPER_LOOP_STREAM_BEGIN
    MSG_MAN_HELPER_STREAM_ACTIVE_BEGIN
        (stream->convert_to_ostream()) << o;
    MSG_MAN_HELPER_STREAM_ACTIVE_END
    MSG_MAN_HELPER_LOOP_STREAM_END
    
    return *this;
}


bool msg_man::open(msg_stream_file*& s, const string& name, const string& filename)
{
    if (s) {
        close(s);
        s->name = name;
        s->file_name = filename;
        s->open();
        return s->enabled();
    }
    
    s = new msg_stream_file(name, filename);
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


string itos(uint64_t v)
{
    char sz[1024];
    sprintf(sz, "%ld", v);
    return string(sz);
}


void msg_man::display_formatted_data(const list<int>& data, bool hex, int max_chars, char fill, bool insert_0x, int sec_num, int row_num, int indent, const char* element_delimiter, const char* sec_delimiter, const char* line_delimiter)
{
    bool start_of_page = true;
    bool start_of_line = true;
    bool start_of_sec = true;
    int  count = 0;

    //MY_DEBUG << "";
    for (list<int>::const_iterator it = data.cbegin(); it != data.cend(); it++, count++) {
        if (!start_of_page) {
            if (start_of_line) {
                if (line_delimiter) MSGX << line_delimiter;
                MSGX << endl;
                for (int j = 0; j < indent; j++) MSGX << " ";
            }
            else if (start_of_sec) {
                if (sec_delimiter) MSGX << sec_delimiter;
            }
            else {
                if (element_delimiter) MSGX << element_delimiter;
            }     
        }
        else {
            for (int j = 0; j < indent; j++) MSGX << " ";
        }
    
        print_1_data(*it, hex, max_chars, fill, insert_0x);

        start_of_page = false;
        if (count % row_num == row_num - 1) {
            start_of_line = true;
            start_of_sec = true;
        }
        else if (count % sec_num == sec_num - 1) {
            start_of_line = false;
            start_of_sec = true;
        }
        else {
            start_of_line = false;
            start_of_sec = false;
        }
    }
    MSGX << endl;
}


void msg_man::display_formatted_data(const unsigned char* data, int len, bool hex, int max_chars, char fill, bool insert_0x, int sec_num, int row_num, int indent, const char* element_delimiter, const char* sec_delimiter, const char* line_delimiter)
{
    bool start_of_page = true;
    bool start_of_line = true;
    bool start_of_sec = true;
    int  count = 0;

    //MY_DEBUG << "";
    for (int i = 0; i < len; i++, count++) {
        if (!start_of_page) {
            if (start_of_line) {
                if (line_delimiter) MSGX << line_delimiter;
                MSGX << endl;
                for (int j = 0; j < indent; j++) MSGX << " ";
            }
            else if (start_of_sec) {
                if (sec_delimiter) MSGX << sec_delimiter;
            }
            else {
                if (element_delimiter) MSGX << element_delimiter;
            }     
        }
        else {
            for (int j = 0; j < indent; j++) MSGX << " ";
        }
    
        print_1_data((int)(data[i]), hex, max_chars, fill, insert_0x);

        start_of_page = false;
        if (count % row_num == row_num - 1) {
            start_of_line = true;
            start_of_sec = true;
        }
        else if (count % sec_num == sec_num - 1) {
            start_of_line = false;
            start_of_sec = true;
        }
        else {
            start_of_line = false;
            start_of_sec = false;
        }
    }
    MSGX << endl;
}



void msg_man::diff_formatted_data(const unsigned char* data0, const unsigned char* data1, int len, bool hex, int max_chars, char fill, bool insert_0x, int sec_num, int row_num, int indent, const char* element_delimiter, const char* sec_delimiter, const char* line_delimiter)
{
    bool start_of_page = true;
    bool start_of_line = true;
    bool start_of_sec = true;
    int  count = 0;

    MSGX << "---- data 0 ---" << endl;
    //MY_DEBUG << "";
    for (int i = 0; i < len; i++, count++) {
        if (!start_of_page) {
            if (start_of_line) {
                if (line_delimiter) MSGX << line_delimiter;
                MSGX << endl;
                for (int j = 0; j < indent; j++) MSGX << " ";
            }
            else if (start_of_sec) {
                if (sec_delimiter) MSGX << sec_delimiter;
            }
            else {
                if (element_delimiter) MSGX << element_delimiter;
            }     
        }
        else {
            for (int j = 0; j < indent; j++) MSGX << " ";
        }
    
        const char* pfx = (data0[i] == data1[i])? " ": ">";
        const char* sfx = (data0[i] == data1[i])? " ": "<";
        print_1_data((int)(data0[i]), hex, max_chars, fill, insert_0x, pfx, sfx);

        start_of_page = false;
        if (count % row_num == row_num - 1) {
            start_of_line = true;
            start_of_sec = true;
        }
        else if (count % sec_num == sec_num - 1) {
            start_of_line = false;
            start_of_sec = true;
        }
        else {
            start_of_line = false;
            start_of_sec = false;
        }
    }
    MSGX << endl;
    
    MSGX << "---- data 1 ---" << endl;
    string element_delimiter1 = element_delimiter;
    element_delimiter1 += "  ";
    string sec_delimiter1 = sec_delimiter;
    sec_delimiter1 += "  ";
    display_formatted_data(data1, len, hex, max_chars, fill, insert_0x, sec_num, row_num, indent + 1, element_delimiter1.c_str(), sec_delimiter1.c_str(), line_delimiter);
}


