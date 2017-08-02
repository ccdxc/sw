#ifndef __THREAD_HPP__
#define __THREAD_HPP__

class thread {
public:
    thread();
    ~thread();
    start();
    stop();

private:
    // TODO: thread's message queue
    bool    running_;
};

#endif    // __THREAD_HPP__
