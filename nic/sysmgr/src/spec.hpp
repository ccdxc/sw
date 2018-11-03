#ifndef __SPEC_H__
#define __SPEC_H__

#include <string>
#include <vector>

using namespace std;

#define  DEFAULT_SPEC_FLAGS 0x0000
#define  RESTARTABLE        0x0001
#define  NO_WATCHDOG        0x0002
#define  NON_CRITICAL       0x0004

class Spec {
    public:
        Spec(const string name, int flags,
            const string command, const vector<string> dependencies):
            name(name), command(command), dependencies(dependencies), 
            flags(flags) {
        }
        const string name;
        const string command;
        const vector<string> dependencies;
        const int flags;
};

extern vector<Spec> specs_from_json(const char *filename);

#endif
