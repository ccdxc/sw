#! /usr/bin/python3
import iota.harness.api as api
import iota.test.utils.p4ctl as p4ctl

###
# Test List subcommands
###
def test_list_cmd(node_name):
    api.Logger.info("Testing p4ctl List command")
    failed = False
    out = p4ctl.RunP4ctlCmd_LIST(node_name)
    if not out:
        api.Logger.error("Failed to run the List command on %s"%(node_name))
        return api.types.status.FAILURE

    tables = p4ctl.GetTables(node_name)
    if not len(tables):
        api.Logger.error("Failed to get the list of tables on %s"%(node_name))
        return api.types.status.FAILURE

    for table in tables:
        if table not in out:
            api.Logger.error("Failed to find table: %s in output"%(table))
            failed = True

    return api.types.status.FAILURE if failed else api.types.status.SUCCESS

###
# Test Read subcommands
###
def test_read_help_cmd(node_name):
    api.Logger.info("Testing p4ctl Read Help command")
    failed = False
    req = api.Trigger_CreateExecuteCommandsRequest(serial=False)

    p4ctl.AddP4CtlCmd_READ(req, node_name, "--help ")
    resp = api.Trigger(req)
    if not resp:
        api.Logger.error("P4ctl Read command trigger failed.")
        return api.types.status.FAILURE

    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error("Failed to test Read help command")
            failed = True
    return api.types.status.FAILURE if failed else api.types.status.SUCCESS

def test_read_table_help_cmd(node_name, table_name=None):
    failed = False
    req = api.Trigger_CreateExecuteCommandsRequest(serial=False)
    tables = [table_name] if table_name else p4ctl.GetTables(node_name)

    if not len(tables):
        api.Logger.error("Failed to get the list of tables on %s"%(node_name))
        return api.types.status.FAILURE

    for table in tables:
        api.Logger.info("Testing p4ctl Read %s Help command"%(table))
        p4ctl.AddP4CtlCmd_READ_TABLE(req, node_name, table, "--help")

    api.Logger.info("This may take little longer, hang in there.")
    resp = api.Trigger(req)
    if not resp:
        api.Logger.error("P4ctl Read command trigger failed.")
        return api.types.status.FAILURE

    for i,cmd in enumerate(resp.commands):
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error("Failed to test Read %s Help command"%tables[i])
            failed = True

    return api.types.status.FAILURE if failed else api.types.status.SUCCESS

def test_read_table_index_cmd(node_name, table_name=None, index=0):
    failed = False
    req = api.Trigger_CreateExecuteCommandsRequest(serial=False)
    tables = [table_name] if table_name else p4ctl.GetTables(node_name)

    if not len(tables):
        api.Logger.error("Failed to get the list of tables on %s"%(node_name))
        return api.types.status.FAILURE

    for table in tables:
        api.Logger.info("Testing p4ctl Read %s Index %s command"%(table, index))
        params = ["%s_index"%(table), "%s"%(index)]
        p4ctl.AddP4CtlCmd_READ_TABLE(req, node_name, table, " ".join(params))

    api.Logger.info("This may take little longer, hang in there.")
    resp = api.Trigger(req)
    if not resp:
        api.Logger.error("P4ctl Read command trigger failed.")
        return api.types.status.FAILURE

    for i,cmd in enumerate(resp.commands):
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error("Failed to test Read %s table index %s"%(tables[i], index))
            failed = True

    return api.types.status.FAILURE if failed else api.types.status.SUCCESS

###
# Test Write subcommands
###
def test_write_help_cmd(node_name):
    failed = False
    api.Logger.info("Testing p4ctl Write Help command")
    req = api.Trigger_CreateExecuteCommandsRequest(serial=False)
    p4ctl.AddP4CtlCmd_WRITE(req, node_name, "--help")
    resp = api.Trigger(req)
    if not resp:
        api.Logger.error("P4ctl Write command trigger failed.")
        return api.types.status.FAILURE

    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error("Failed to test Write help command")
            failed = True
    return api.types.status.FAILURE if failed else api.types.status.SUCCESS

def test_write_table_help_cmd(node_name, table_name=None):
    failed = False
    req = api.Trigger_CreateExecuteCommandsRequest(serial=False)
    tables = [table_name] if table_name else p4ctl.GetTables(node_name)

    if not len(tables):
        api.Logger.error("Failed to get the list of tables on %s"%(node_name))
        return api.types.status.FAILURE

    for table in tables:
        api.Logger.info("Testing p4ctl Write %s Help command"%(table))
        p4ctl.AddP4CtlCmd_WRITE_TABLE(req, node_name, table, "--help")

    api.Logger.info("This may take little longer, hang in there.")
    resp = api.Trigger(req)
    if not resp:
        api.Logger.error("P4ctl Write command trigger failed.")
        return api.types.status.FAILURE

    for i,cmd in enumerate(resp.commands):
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error("Failed to test Write %s help command."%tables[i])
            failed = True

    return api.types.status.FAILURE if failed else api.types.status.SUCCESS
