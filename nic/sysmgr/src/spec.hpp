#ifndef __SPEC_H__
#define __SPEC_H__

#include <string>
#include <vector>

using namespace std;

enum flags {
    DEFAULT_SPEC_FLAGS = 0x0000,
    RESTARTABLE        = 0x0001,
    NO_WATCHDOG        = 0x0002,
};

class Spec {
    public:
        Spec(const string name, enum flags flags,
            const string command, const vector<string> dependencies):
            name(name), command(command), dependencies(dependencies), 
            flags(flags) {
        }
        const string name;
        const string command;
        const vector<string> dependencies;
        const enum flags flags;
};

extern const vector<Spec> SPECS;

#endif
