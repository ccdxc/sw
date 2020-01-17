#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.verif.utils.rdma_utils as rdma
import iota.test.utils.naples_host as host

def Setup(tc):
    api.SetTestsuiteAttr("driver_path", api.GetHostToolsDir() + '/')
    tc.iota_path = api.GetTestsuiteAttr("driver_path")

    tc.nodes = api.GetNaplesHostnames()
    tc.other_nodes = api.GetWorkloadNodeHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])

    platform_gendir = api.GetTopDir()+'/platform/gen/'
    if tc.os == host.OS_TYPE_LINUX:
        tc.pkgname = 'drivers-linux.tar.xz'
        tc.showgid = 'drivers-linux/show_gid'
    else:
        tc.pkgname = 'drivers-freebsd.tar.xz'
        tc.showgid = 'drivers-freebsd/show_gid'

    # Copy RDMA driver to naples nodes
    for n in tc.nodes:
        api.Logger.info("Copying {pkg} to {node}"
                .format(pkg=tc.pkgname, node=n))
        resp = api.CopyToHost(n, [platform_gendir + tc.pkgname])
        if not api.IsApiResponseOk(resp):
            api.Logger.error("Failed to copy {pkg} to {node}: {resp}"
                    .format(pkg=tc.pkgname, node=n, resp=resp))
            return api.types.status.FAILURE

    # Copy show_gid to other nodes
    for n in tc.other_nodes:
        if n in tc.nodes:
            continue
        api.Logger.info("Copying show_gid to {node}"
                .format(node=n))
        resp = api.CopyToHost(n, [platform_gendir + tc.showgid])
        if not api.IsApiResponseOk(resp):
            api.Logger.error("Failed to copy show_gid to {node}: {resp}"
                    .format(node=n, resp=resp))
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Trigger(tc):
    req_uname = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    # Move driver package into position and build on naples nodes
    for n in tc.nodes:
        api.Logger.info("Building RDMA drivers and tools on: {node}".format(node=n))
        if tc.os == host.OS_TYPE_LINUX:
            api.Trigger_AddHostCommand(req, n,
                    "mkdir -p {path} && mv {pkg} {path} && cd {path} && tar -xma --strip-components 1 -f {pkg} && ./setup_libs.sh && ./build.sh"
                    .format(path=tc.iota_path, pkg=tc.pkgname),
                    timeout = 180)
        else:
            api.Trigger_AddHostCommand(req, n,
                    "mkdir -p {path} && mv {pkg} {path} && cd {path} && tar -xm --strip-components 1 -f {pkg} && ./build.sh"
                    .format(path=tc.iota_path, pkg=tc.pkgname),
                    timeout = 180)
        api.Trigger_AddHostCommand(req_uname, n, "uname -r")

    # Move show_gid into position on other nodes
    for n in tc.other_nodes:
        if n in tc.nodes:
            continue
        api.Logger.info("Moving show_gid to tools on {node}"
                .format(node=n))
        api.Trigger_AddHostCommand(req, n, "mkdir -p {path} && mv show_gid {path}"
                .format(path=tc.iota_path))
        api.Trigger_AddHostCommand(req_uname, n, "uname -r")

    tc.resp_uname = api.Trigger(req_uname)
    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp_uname is None or tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS

    api.Logger.info("build_rdma results for the following nodes: {0}".format(tc.nodes))

    unames = []
    for cmd in tc.resp_uname.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE
        else:
            unames.append(cmd.stdout)

    api.SetTestsuiteAttr("unames", unames)

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE

    return result

def Teardown(tc):
    #set the path for testcases in this testsuite to use
    return api.types.status.SUCCESS
