#include <sys/time.h>

#include "lib/operd/operd.hpp"
#include "operdctl.hpp"

static char
level_to_c (uint8_t level) {
    switch (level) {
    case 1:
        return 'E';
    case 2:
        return 'W';
    case 3:
        return 'I';
    case 4:
        return 'D';
    default:
        return 'T';
    }
}

int
dump (int argc, const char *argv[])
{
    sdk::operd::consumer_ptr consumer;
    sdk::operd::log_ptr log;
    char timebuf[256];
    const struct timeval *ts;
    struct tm *ltm;
    
    if (argc != 1) {
        printf("Usage: dump <region>\n");
        return -1;
        
    }

    consumer = sdk::operd::consumer::create(argv[0]);
    consumer->reset();

    while (true) {
        log = consumer->read();
        if (log == nullptr) {
            return 0;
        }
        
        ts = log->timestamp();
        ltm = localtime(&ts->tv_sec);
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", ltm);
  
        printf("%c [%s.%06ld] %i: %s\n", level_to_c(log->severity()), timebuf,
               ts->tv_usec, log->pid(), log->data());       
    }
    
    return 0;
}
