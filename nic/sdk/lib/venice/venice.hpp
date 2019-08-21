// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __VENICE_HPP__
#define __VENICE_HPP__

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include "include/sdk/base.hpp"
#include "include/sdk/types.hpp"
#include <map>

namespace sdk {
namespace lib {

using boost::property_tree::ptree;
using namespace std;

#define DEFAULT_VENICE_FILE0    "/sysconfig/config0/naplesStatus.json"
#define DEFAULT_VENICE_FILE1    "/sysconfig/config1/naplesStatus.json"

typedef struct fru_s {
    string manufacturingDate;
    string manufacturer;
    string productName;
    string serialNum;
    string partNum;
    string boardId;
    string engChangeLevel;
    string numMacAddr;
    string macStr;
} fru_t;

typedef struct venice_s {
    string naples_status_file;
    vector<string> controllers;
    string naplesMode;
    string transitionPhase;
    string id;
    string mgmtIp;
    string smartNicName;
    fru_t  fru;
} venice_t;

class venice {
public:
    venice_t venice_db_;    // device database

    static venice *factory(std::string naples_status_file="");
    static void destroy(venice *dev);
    sdk_ret_t init(std::string &naples_status_file);
    sdk_ret_t dump_venice_coordinates();
    string get_naples_status_file();

private:
    venice() {};
    ~venice();

    static sdk_ret_t get_ptree_(std::string& naples_status_file, ptree& prop_tree);
    void populate_venice_defaults_();

    // populate venice
    sdk_ret_t populate_venice(ptree &pt);
};


}    // namespace lib
}    // namespace sdk

#endif    //__VENICE_HPP__
