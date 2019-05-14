#ifndef __FAULT_WATCHER_HPP__
#define __FAULT_WATCHER_HPP__

#include <list>
#include <memory>
#include <string>

#include <ev++.h>

#include "custom_loop.hpp"

class FaultReactor
{
public:
    virtual void on_fault(std::string reason) = 0;
};
typedef std::shared_ptr<FaultReactor> FaultReactorPtr;

class FaultLoop : public CustomLoopReactor
{
private:
    bool fault_set = false;
    bool fault_pending = false;
    std::string fault_reason = "";
    std::list<FaultReactorPtr> fault_reactors;
    static std::shared_ptr<FaultLoop> instance;
public:
    static std::shared_ptr<FaultLoop> getInstance();
    void set_fault(std::string reason);
    void register_fault_reactor(FaultReactorPtr watcher);
    virtual void do_work();
    virtual bool has_work();
};
typedef std::shared_ptr<FaultLoop> FaultLoopPtr;

#endif // __FAULT_WATCHER_HPP__
