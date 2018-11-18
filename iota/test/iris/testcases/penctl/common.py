import iota.harness.api as api


PENCTL_ROOT_DIR   = 'penctl_bin'
PENCTL_DEST_DIR   = 'nic/bin/'

PENCTL_PKG = "nic/host.tar"
PENCTL_NAPLES_PKG = "nic/naples_fw.tar"

def GetNaplesMgmtIP(node):
    return  "1.0.0.2"

def GetNaplesTunIntf(node):
    return  "tun0"

def __get_pen_ctl_cmd(node):
    mgmt_intf = GetNaplesTunIntf(node)
    if api.GetNodeOs(node) == "linux":
        return "PENETHDEV=%s ./penctl.linux " % (mgmt_intf)
    elif api.GetNodeOs(node) == "freebsd":
        return "PENETHDEV=%s ./penctl.freebsd " % (mgmt_intf)
    else:
        api.Logger.error("Invalid node os type")
        assert(0)

def AddPenctlCommand(req, node, cmd):
    cur_dir = api.GetCurrentDirectory()
    api.ChangeDirectory(PENCTL_ROOT_DIR)
    api.Trigger_AddHostCommand(req, node, __get_pen_ctl_cmd(node) + cmd, background = False,
                               rundir = PENCTL_DEST_DIR, timeout = 60*120)
    api.ChangeDirectory(cur_dir)
