#ifndef __SPEC_H__
#define __SPEC_H__

#include <string>
#include <vector>

using namespace std;

enum spec_restartability {
    NON_RESTARTABLE,
    RESTARTABLE,
};

class Spec {
    public:
        Spec(const string name, enum spec_restartability restartability,
            const string command, const vector<string> dependencies):
            name(name), command(command), dependencies(dependencies), 
            restartability(restartability) {
        }
        const string name;
        const string command;
        const vector<string> dependencies;
        const enum spec_restartability restartability;
};

extern const vector<Spec> SPECS;

#endif
