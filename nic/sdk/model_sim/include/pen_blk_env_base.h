#ifndef _PEN_BLK_ENV_BASE_H_
#define _PEN_BLK_ENV_BASE_H_
#include <string>

class pen_blk_test_base;

class pen_blk_env_base {
    protected:
        int _id;
        pen_blk_test_base * test;

    public:
        pen_blk_env_base(int id);
        virtual ~pen_blk_env_base();
        virtual void add_test  (const std::string &name);
        virtual void exec_phase(const std::string &name);

};

#endif
