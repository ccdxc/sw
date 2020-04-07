#include "Config.h"

#include <winioctl.h>
#include <shlwapi.h>

#include "UserCommon.h"
#include "ionic_types.h"

static
po::options_description
CmdPortOpts()
{
    po::options_description opts("IonicConfig.exe [-h] Port [options ...]");

    opts.add_options()
        ("Name,n", optype_wstring()->required(), "Interface name")
        ("AutoNeg,a", optype_bool(), "Set Auto Negotiation (yes|no)")
        ("Speed,s", optype_string(), "Set Speed (Mbps|Ndis link speed enum)")
        ("FEC,f", optype_string(), "Set FEC (None|FC|RS)")
        ("Pause,u", optype_string(), "Set Pause (None|TX|RX|Both)")
        ("PauseType,t", optype_string(), "Set Pause Type (Link|PFC) (Pause != None)");

    return opts;
}

static
int
CmdPortRun(command_info& info)
{
    if (info.usage) {
        std::cout << info.cmd.opts() << info.cmd.desc << std::endl;
        return info.status;
    }

    PortSetCB cb = {};

    const std::wstring& name = opval_wstring(info.vm, "Name");

    opval_wstrncpy(cb.AdapterName, sizeof(cb.AdapterName), info.vm, "Name");

    if (info.vm.count("AutoNeg")) {
        cb.Flags |= PORT_SET_AUTONEG;
        cb.Config.AutoNeg = opval_bool(info.vm, "AutoNeg");
    }

    if (info.vm.count("Speed")) {
        cb.Flags |= PORT_SET_SPEED;
        cb.Config.Speed = opval_long(info.vm, "Speed");
    }

    if (info.vm.count("FEC")) {
        cb.Flags |= PORT_SET_FEC;
        const std::string& fec = opval_string(info.vm, "FEC");

        // XXX ionic_if.h: PORT_FEC_TYPE_NONE/FC/RS
        if (boost::iequals(fec, "None")) {
            cb.Config.FEC = 0;
        } else if (boost::iequals(fec, "FC")) {
            cb.Config.FEC = 1;
        } else if (boost::iequals(fec, "RS")) {
            cb.Config.FEC = 2;
        } else {
            invalid_option("FEC", fec);
        }
    }

    if (info.vm.count("Pause")) {
        cb.Flags |= PORT_SET_PAUSE;
        const std::string& pause = opval_string(info.vm, "Pause");

        if (boost::iequals(pause, "None")) {
            // if Pause is None, then any PauseType is invalid
            if (info.vm.count("PauseType")) {
                invalid_option("PauseType", pause);
            }
        } else {
            // XXX ionic_if.h: IONIC_PAUSE_F_TX/RX
            if (boost::iequals(pause, "TX")) {
                cb.Config.Pause |= 0x10;
            } else if (boost::iequals(pause, "RX")) {
                cb.Config.Pause |= 0x20;
            } else if (boost::iequals(pause, "Both")) {
                cb.Config.Pause |= 0x30;
            } else {
                invalid_option("Pause", pause);
            }

            // if Pause is not None, then PauseType is required
            if (!info.vm.count("PauseType")) {
                throw po::required_option("Pause");
            }

            const std::string& pause_type = opval_string(info.vm, "PauseType");

            // XXX ionic_if.h: IONIC_PAUSE_TYPE_LINK/PFC
            if (boost::iequals(pause_type, "Link")) {
                cb.Config.Pause |= 1;
            } else if (boost::iequals(pause_type, "PFC")) {
                cb.Config.Pause |= 2;
            } else {
                invalid_option("PauseType", pause_type);
            }
        }
    } else {
        // if PauseType is given, then Pause is required
        if (info.vm.count("PauseType")) {
            throw po::required_option("Pause");
        }
    }

    if (cb.Flags) {
        if (!DoIoctl(IOCTL_IONIC_PORT_SET, &cb, sizeof(cb), NULL, 0)) {
            info.status = 1;
        }
    } else {
        if (!DoIoctl(IOCTL_IONIC_PORT_GET,
                     cb.AdapterName, sizeof(cb.AdapterName),
                     &cb.Config, sizeof(cb.Config))) {
            info.status = 1;
        } else {
            std::cout << "Port Configuration: " << to_bytes(name) << std::endl;

            std::cout << std::left << std::setw(20) << "AutoNeg: "
                << (cb.Config.AutoNeg ? "yes" : "no") << std::endl;

            std::cout << std::left << std::setw(20) << "Speed: "
                << cb.Config.Speed << " Mbps" << std::endl;

            // XXX ionic_if.h: PORT_FEC_TYPE_NONE/FC/RS
            std::cout << std::left << std::setw(20) << "FEC: ";
            if (cb.Config.FEC == 0) {
                std::cout << "None" << std::endl;
            } else if (cb.Config.FEC == 1) {
                std::cout << "FC" << std::endl;
            } else if (cb.Config.FEC == 2) {
                std::cout << "RS" << std::endl;
            } else {
                std::cout << cb.Config.FEC << " (unknown)" << std::endl;
            }

            // XXX ionic_if.h: IONIC_PAUSE_F_TX/RX
            std::cout << std::left << std::setw(20) << "Pause: ";
            if ((cb.Config.Pause & 0xf0) == 0) {
                std::cout << "None" << std::endl;
            } else if ((cb.Config.Pause & 0xf0) == 0x10) {
                std::cout << "TX" << std::endl;
            } else if ((cb.Config.Pause & 0xf0) == 0x20) {
                std::cout << "RX" << std::endl;
            } else if ((cb.Config.Pause & 0xf0) == 0x30) {
                std::cout << "Both" << std::endl;
            } else {
                std::cout << std::hex << (cb.Config.Pause & 0xf0) << " (unknown)" << std::endl;
            }

            // XXX ionic_if.h: IONIC_PAUSE_TYPE_NONE/LINK/PFC
            std::cout << std::left << std::setw(20) << "PauseType: ";
            if ((cb.Config.Pause & 0x0f) == 0) {
                std::cout << "None" << std::endl;
            } else if ((cb.Config.Pause & 0x0f) == 1) {
                std::cout << "Link" << std::endl;
            } else if ((cb.Config.Pause & 0x0f) == 2) {
                std::cout << "PFC" << std::endl;
            } else {
                std::cout << std::hex << (cb.Config.Pause & 0x0f) << " (unknown)" << std::endl;
            }
        }
    }

    return info.status;
}

command
CmdPort()
{
    command cmd;

    cmd.name = "Port";
    cmd.desc = "Query or set port configuration";

    cmd.opts = CmdPortOpts;
    cmd.run = CmdPortRun;

    return cmd;
}
