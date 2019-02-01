#include <ctime>
#include "msg_stream.h"
#include "msg_man.h"
#include "common_dpi.h"

void msg_stream::set_scope(const string& scope_name, bool deep)
{
    man.scope_root->remove_stream(this, true);
    msg_scope* s = man.create_or_get_scope(scope_name);
    s->insert_stream(this, deep || deep_scope);
}


void msg_stream::add_scope(const string& scope_name, bool deep)
{
    msg_scope* s = man.create_or_get_scope(scope_name);
    s->insert_stream(this, deep || deep_scope);
}


void msg_stream::remove_scope(const string& scope_name, bool deep)
{
    msg_scope* s = man.create_or_get_scope(scope_name);
    s->remove_stream(this, deep || deep_scope);
}


void msg_stream::set_level(const string& scope_name, msg_level_t level, bool deep)
{
    msg_scope* s = man.create_or_get_scope(scope_name);
    if (!deep && !deep_scope)  {
        if (scopes.find(s) != scopes.end()) {
            levels[s] = level;
        }
    }
    else {
        set_level_recursive(s, level);
    }
}


void msg_stream::set_high_level(const string& scope_name, msg_level_t level, bool deep)
{
    msg_scope* s = man.create_or_get_scope(scope_name);
    if (!deep && !deep_scope)  {
        if (scopes.find(s) != scopes.end()) {
            high_levels[s] = level;
        }
    }
    else {
        set_high_level_recursive(s, level);
    }
}


void msg_stream::set_level_recursive(msg_scope* scope, msg_level_t level)
{
    if (scopes.find(scope) != scopes.end()) {
        levels[scope] = level;
    }
    for (vector<msg_scope*>::iterator it = scope->children.begin(); it != scope->children.end(); it++) {
        set_level_recursive(*it, level);
    }
}



void msg_stream::set_high_level_recursive(msg_scope* scope, msg_level_t level)
{
    if (scopes.find(scope) != scopes.end()) {
        high_levels[scope] = level;
    }
    for (vector<msg_scope*>::iterator it = scope->children.begin(); it != scope->children.end(); it++) {
        set_high_level_recursive(*it, level);
    }
}



msg_stream::msg_stream(msg_man& m, const string& name)
    : man(m)
{
    this->name = name;
    set_insert_level(true);
    set_insert_scope(false);
#ifdef _CSV_INCLUDED_
    set_insert_time(true);
#else 
    set_insert_time(false);
#endif
    set_insert_name(false);
    deep_scope = true;
    enable = true;
    default_level = msg;
    default_high_level = error;
    tag_inserted = false;
    color = 0;
}


msg_stream::~msg_stream()
{
}


void msg_stream::update_scope()
{
    set<msg_scope*> backup = scopes;
    for (set<msg_scope*>::iterator it = backup.begin(); it != backup.end(); it++) {
        (*it)->insert_stream(this, false);
    }
}


void msg_stream::begin_msg(const msg_scope* scope, msg_level_t level)
{
    if (tag_inserted) return;

#ifdef _CSV_INCLUDED_
    convert_to_ostream() << "[C++]";
#endif

    if (insert_level) {
        convert_to_ostream() << "[" << msg_man::display_level((msg_man::msg_level_t)level) << "]";
    }


    if (insert_name) {
        convert_to_ostream() << "[" << name << "]";
    }


    if (insert_time) {
#ifdef _CSV_INCLUDED_
        char ts[128];
        dpi_sv_get_sim_time_ns(ts);
        convert_to_ostream() << "[" << ts << "]";
#else 
        time_t t = time(0);
        struct tm* now = localtime(&t);
        convert_to_ostream() << dec << "["
            << now->tm_mon << "/" 
            << now->tm_mday << "/"
            << now->tm_year << "-"
            << now->tm_hour << ":"
            << now->tm_min << ":"
            << now->tm_sec << "]";
#endif                                    
    }


    if (insert_scope) {
        convert_to_ostream() << "[" << scope->name << "]";
    }
    
    convert_to_ostream() << " ";
    tag_inserted = true;
}


bool msg_stream::pass_level_test(msg_level_t lvl, const msg_scope* scope)
{
    msg_level_t high_level;
    msg_level_t level;

    if (levels.find(scope) != levels.end()) level = levels[scope];
    else level = default_level;
    if (high_levels.find(scope) != high_levels.end()) high_level = high_levels[scope];
    else high_level = default_high_level;

    if (lvl >= high_level && lvl <= level) {
        color = 1;
        return true;
    }
    else return false;
}


void msg_stream::open() 
{
    set_deep_scope();
    set_scope("/");
    set_level("/", msg);
    enable = true;
}


msg_stream_cout::msg_stream_cout()
    : msg_stream(msg_man::instance(), "cout")
{
}


ostream& msg_stream_cout::convert_to_ostream()
{
    return std::cout;
}


void msg_stream_cout::open()
{
    msg_stream::open();
    set_deep_scope();
    set_scope("/");
    if (MSG_STREAMP_CERR && MSG_STREAMP_CERR->enabled()) set_high_level("/", warning);
    else set_high_level("/", error);
}


msg_stream_cerr::msg_stream_cerr()
    : msg_stream(msg_man::instance(), "cerr")
{
}


ostream& msg_stream_cerr::convert_to_ostream()
{
    return std::cerr;
}


void msg_stream_cerr::open()
{
    msg_stream::open();
    set_deep_scope();
    set_scope("/");
    set_level("/", error);
    if (MSG_STREAMP_COUT && MSG_STREAMP_COUT->enabled()) MSG_STREAMP_COUT->set_high_level("/", warning);
}


void msg_stream_cerr::close()
{
    if (MSG_STREAMP_COUT && MSG_STREAMP_COUT->enabled()) MSG_STREAMP_COUT->set_high_level("/", error);
    msg_stream::close();
}



msg_stream_file::msg_stream_file(const string& name, const string& fname)
    : msg_stream(msg_man::instance(), name)
{
    file_name = fname;
}


ostream& msg_stream_file::convert_to_ostream()
{
    return file_stream;
}


void msg_stream_file::open()
{
    msg_stream::open();

    file_stream.open(file_name.c_str(), ios_base::out | ios_base::trunc);
    if (!file_stream.is_open()) {
        MSG_ERROR_SCOPE("/") << "file '" << file_name << "' can NOT open for write." << endl;
        set_enable(false);
    }
}


void msg_stream_file::close()
{
    if (file_stream.is_open()) {
        file_stream.close();
    }
    msg_stream::close();
}


void msg_stream::save_format()
{
    format = convert_to_ostream().flags();
}


void msg_stream::restore_format()
{
    convert_to_ostream().flags(format);
}

