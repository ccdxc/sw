#pragma once

#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include <boost/process.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/process/async.hpp>
#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <codecvt>
#include <iomanip>
#include <iostream>

namespace po = boost::program_options;
namespace bp = boost::process;

namespace {
    std::string to_bytes(const std::wstring &wstr)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;

        return convert.to_bytes(wstr);
    }

    po::positional_options_description pos_none()
    {
        return po::positional_options_description();
    }

    void invalid_option(const std::string& name, const std::string &val)
    {
        auto e = po::invalid_option_value(val);
        e.add_context(name, name, 0);
        throw e;
    }

    po::typed_value<bool>* optype_flag()
    {
        return po::value<bool>()->composing()->implicit_value(false)->zero_tokens();
    }

    po::typed_value<bool>* optype_bool()
    {
        return po::value<bool>();
    }

    bool opval_bool(const po::variables_map& vm, const std::string& name)
    {
        return vm[name].as<bool>();
    }

    po::typed_value<std::string>* optype_long()
    {
        return po::value<std::string>();
    }

    long string_long(const std::string& sval, const std::string& name)
    {
        size_t pos = 0;
        long val;

        try {
            val = std::stol(sval, &pos, 0);
        }
        catch (...) {
            pos = -1;
        }

        if (pos != sval.length()) {
            invalid_option(name, sval);
        }

        return val;
    }

    long opval_long(const po::variables_map& vm, const std::string& name)
    {
        const std::string &sval = vm[name].as<std::string>();

        return string_long(sval, name);
    }

    po::typed_value<std::string>* optype_string()
    {
        return po::value<std::string>();
    }

    const std::string & opval_string(const po::variables_map& vm, const std::string& name)
    {
        return vm[name].as<std::string>();
    }

    po::typed_value<std::wstring, wchar_t>* optype_wstring()
    {
        return po::wvalue<std::wstring>();
    }

    const std::wstring & opval_wstring(const po::variables_map& vm, const std::string& name)
    {
        return vm[name].as<std::wstring>();
    }

    void opval_wstrncpy(wchar_t* wstr, size_t size, const po::variables_map& vm, const std::string& name)
    {
        const std::wstring& wsval = opval_wstring(vm, name);
        const std::string sval = to_bytes(wsval);
        if (sval.length() >= size) {
            invalid_option(name, sval);
        }
        else {
            wstr[wsval.copy(wstr, size - 1)] = 0;
        }
    }
}

struct command_info;

struct command {
    std::string name;
    std::string desc;
    bool hidden;
    po::options_description(*opts)(bool hidden);
    po::positional_options_description(*pos)();
    int (*run)(command_info& info);

    command() : hidden(), opts(), pos(pos_none), run() {}
};

struct command_info {
    int status;
    bool usage;
    bool hidden;
    bool dryrun;

    std::string cmd_name;
    std::vector<std::wstring> cmd_args;
    po::variables_map vm;

    command cmd;
    std::vector<command> cmds;

    command_info() : status(), usage(), hidden(), dryrun() {}
};

DWORD
DoIoctl(DWORD dwIoControlCode,
        LPVOID lpInBuffer,
        DWORD nInBufferSize,
        LPVOID lpOutBuffer,
        DWORD nOutBufferSize,
        PDWORD pnBytesReturned,
        bool dryrun);

command CmdUsage();
command CmdVersion();
command CmdSetTrace();
command CmdGetTrace();
command CmdPort();
command CmdRxBudget();
command CmdDevStats();
command CmdLifStats();
command CmdPortStats();
command CmdPerfStats();
command CmdRegKeys();
command CmdInfo();
//TODO command CmdOidStats();
//TODO command CmdFwcmdStats();

DWORD
OptGetAlternateNames(command_info& info,
				  WCHAR *name,
				  DWORD name_sz,
				  WCHAR *ifname,
				  DWORD ifname_sz,
				  WCHAR *descrname,
				  DWORD descrname_sz);

void
OptGetDevName(command_info& info,
	WCHAR *devname,
	DWORD devname_sz,
	BOOL required);

void
OptAddDevName(po::options_description& opts);
