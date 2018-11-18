#include <algorithm>
#include <ctime>
#include <memory>
#include <string>
#include <utility>

#include "watchdog.hpp"

// Warning: Todo: Fixme:
// The current implementation is slow O(N) for every operation
// It's only an initial implementation and should be changed to something
// better soon

time_t now() {
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return tp.tv_sec;
}

Watchdog::Watchdog() {

}

void Watchdog::refresh(std::string name) {
    for (auto item: this->timeouts) {
        if (item->name == name) {
            item->expires = now() + TIMEOUT;
            return;
        }
    }
    // name was not in the list, add it
    auto item = std::make_shared<struct timeout>(timeout{name, now() + TIMEOUT});
    this->timeouts.push_back(item);
}

std::list<std::string> Watchdog::expired() {
    auto exp = std::list<std::string>();

    time_t current = now();
    for (auto item: this->timeouts) {
        if (current >= item->expires) {
            exp.push_back(item->name);
        }
    }

    return exp;
}

int Watchdog::next_tick() {
    auto expired = std::list<std::string>();

    time_t current = now();
    time_t next_tick = current + TIMEOUT; 
    for (auto item: this->timeouts) {
        if (current >= item->expires) {
            continue;
        }
        next_tick = std::min(next_tick, item->expires);
    }

    return (next_tick - current);
}

void Watchdog::dump() {
    time_t current = now();
    for (auto item: this->timeouts) {
        printf("(%s) expires in %lu sec\n", item->name.c_str(), item->expires - current);
    }
}
