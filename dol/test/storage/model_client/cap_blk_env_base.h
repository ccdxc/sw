#ifndef _CAP_BLK_ENV_BASE_H_
#define _CAP_BLK_ENV_BASE_H_
#include <string>

class cap_blk_test_base;

class cap_blk_env_base {
    protected:
        int _id;
        cap_blk_test_base * test;

    public:
        cap_blk_env_base(int id);
        virtual ~cap_blk_env_base();
        virtual void add_test  (const std::string &name);
        virtual void exec_phase(const std::string &name);

};

#endif
