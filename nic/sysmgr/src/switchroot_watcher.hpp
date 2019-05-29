#ifndef __SWITCHROOT_WATCHER_HPP__
#define __SWITCHROOT_WATCHER_HPP__

#include <list>
#include <memory>
#include <string>

#include <ev++.h>

#include "custom_loop.hpp"

class SwitchrootReactor
{
public:
    virtual void on_switchroot() = 0;
};
typedef std::shared_ptr<SwitchrootReactor> SwitchrootReactorPtr;

class SwitchrootLoop : public CustomLoopReactor
{
private:
    bool switchroot_pending = false;
    std::list<SwitchrootReactorPtr> switchroot_reactors;
    static std::shared_ptr<SwitchrootLoop> instance;
public:
    static std::shared_ptr<SwitchrootLoop> getInstance();
    void set_switchroot();
    void register_switchroot_reactor(SwitchrootReactorPtr watcher);
    virtual void do_work();
    virtual bool has_work();
};
typedef std::shared_ptr<SwitchrootLoop> SwitchrootLoopPtr;

#endif // __SWITCHROOT_WATCHER_HPP__
