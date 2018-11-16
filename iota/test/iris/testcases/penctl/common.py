import iota.harness.api as api


PENCTL_ROOT_DIR   = 'penctl_bin'
PENCTL_DEST_DIR   = 'nic/bin/'

PENCTL_PKG = "nic/host.tar"

def GetNaplesMgmtIP(node):
    return  "1.0.0.2"

def __get_pen_ctl_cmd(node):
    if api.GetNodeOs(node) == "linux":
        return "./penctl.linux "
    elif api.GetNodeOs(node) == "freebsd":
        return "./penctl.freebsd "
    else:
        api.Logger.error("Invalid node os type")
        assert(0)

def AddPenctlCommand(req, node, cmd):
    cur_dir = api.GetCurrentDirectory()
    api.ChangeDirectory(PENCTL_ROOT_DIR)
    api.Trigger_AddHostCommand(req, node, __get_pen_ctl_cmd(node) + cmd, background = False,
                               rundir = PENCTL_DEST_DIR)
    api.ChangeDirectory(cur_dir)
