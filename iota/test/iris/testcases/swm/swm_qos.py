#! /usr/bin/python3
import iota.harness.api as api
from .utils.ncsi_ops import check_set_ncsi

def TriggerSWMQoSConfigVerfication(req, tc):

    api.Logger.info("Running commands to verify SWM QoS Configuration")

    tc.cmd_descr = "QoS configuration commands"


    # channel that has RX mode set is the uplink that is configured to receive BMC traffic
    ncsi_cmd = "/nic/bin/halctl show ncsi channel"
    api.Trigger_AddNaplesCommand(req, 
                                 tc.node_name, 
                                 ncsi_cmd)
    tc.cmd_cookies.append(ncsi_cmd)


    # 0 - NONE; 1 - ETHERTYPE; 2 - DA
    reg_name = 'pb_pbc_hbm_hbm_port_0_cfg_hbm_parser_cam_enable'
    reg_read_cmd = '/bin/echo \'read ' + reg_name + '\' | LD_LIBRARY_PATH=/nic/lib:/platform/lib:$LD_LIBRARY_PATH /platform/bin/capview'

    api.Trigger_AddNaplesCommand(req, 
                                 tc.node_name, 
                                 reg_read_cmd)
    tc.cmd_cookies.append(reg_read_cmd)

    reg_name = 'pb_pbc_hbm_hbm_port_1_cfg_hbm_parser_cam_enable'
    reg_read_cmd = '/bin/echo \'read ' + reg_name + '\' | LD_LIBRARY_PATH=/nic/lib:/platform/lib:$LD_LIBRARY_PATH /platform/bin/capview'

    api.Trigger_AddNaplesCommand(req, 
                                 tc.node_name, 
                                 reg_read_cmd)
    tc.cmd_cookies.append(reg_read_cmd)

    reg_name = 'pb_pbc_hbm_hbm_port_8_cfg_hbm_parser_cam_enable'
    reg_read_cmd = '/bin/echo \'read ' + reg_name + '\' | LD_LIBRARY_PATH=/nic/lib:/platform/lib:$LD_LIBRARY_PATH /platform/bin/capview'

    api.Trigger_AddNaplesCommand(req, 
                                 tc.node_name, 
                                 reg_read_cmd)
    tc.cmd_cookies.append(reg_read_cmd)

    #==============================================================
    # trigger the request
    #==============================================================
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)

    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    return api.types.status.SUCCESS

def VerifySWMQoSConfigVerfication(tc):

    cookie_idx = 0
    swm_uplink = -1
    reg_name = 'pb_pbc_hbm_hbm_port_8_cfg_hbm_parser_cam_enable'
    field_name = 'entry_0'
    result = api.types.status.SUCCESS

    api.Logger.info("Results for {}".format(tc.cmd_descr))

    for cmd in tc.resp.commands:
        api.Logger.info("{}".format(tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)

        if cmd.exit_code != 0:
            result = api.types.status.FAILURE

        if('ncsi channel' in tc.cmd_cookies[cookie_idx]):
            '''
            # halctl show ncsi channel

            Ch: Channel             Channel Modes: TX, RX
            VEN: Vlan Enable (0: Native, 1: Mode is valid
            Vlan-Mode: 0: NATIVE0, 1: VLAN_ONLY, 2: VLAN_NATIVE, 
                       3: ANY_VLAN_NATIVE, 4 - 0xff: NATIVE4
            ----------------------------------------
            Ch   Modes     VEN       Vlan-Mode      
            ----------------------------------------
            0    TX,RX     false     NATIVE0        
            1    -         false     NATIVE0        
            '''

            lines = cmd.stdout.split('\n')
            for line in lines:
                if 'RX' in line:
                    if 'Channel' in line:
                        #banner
                        continue
                    else:
                        line_attrs = line.split()
                        swm_uplink = int(line_attrs[0])
                        api.Logger.info("SWM uplink: {}".format(swm_uplink))

        else:
            '''
            # /bin/echo 'read pb_pbc_hbm_hbm_port_0_cfg_hbm_parser_cam_enable' | LD_LIBRARY_PATH=/nic/lib:/platform/lib:$LD_LIBRARY_PATH /platform/bin/capview
            0x0140c3a8: pb_pbc_hbm_hbm_port_0_cfg_hbm_parser_cam_enable
              val: 0x2
              Fields:
                [  7:6  ] entry_3: 0x0
                [  5:4  ] entry_2: 0x0
                [  3:2  ] entry_1: 0x0
                [  1:0  ] entry_0: 0x2
            '''

            lines = cmd.stdout.split('\n')
            for line in lines:
                if field_name in line:
                    line_attrs = line.split()
                    cam_en = int(line_attrs[len(line_attrs)-1], 0)

                    if(reg_name in tc.cmd_cookies[cookie_idx]):
                        if(cam_en != 1):        #ethertype for OOB port
                            api.Logger.error("cam_en {} is not programmed to ETHERTYPE for OOB port".format(cam_en))
                            result = api.types.status.FAILURE
                        else:
                            api.Logger.info("cam_en {} programmed to ETHERTYPE for OOB port".format(cam_en))
                    else:
                        if((swm_uplink != 0) or (swm_uplink != 1)):
                            api.Logger.error("Invalid swm_uplink {}".format(swm_uplink))
                            result = api.types.status.FAILURE

                        reg = 'pb_pbc_hbm_hbm_port_' + str(swm_uplink) + '_cfg_hbm_parser_cam_enable'
                        if(reg in tc.cmd_cookies[cookie_idx]):
                            if(cam_en != 2):        #da for uplink port
                                api.Logger.error("cam_en {} is not programmed to DMAC for uplink port".format(cam_en))
                                result = api.types.status.FAILURE
                            else:
                                api.Logger.info("cam_en {} programmed to DMAC for uplink port {}".format(cam_en, swm_uplink))


    cookie_idx += 1

    return result

def Setup(tc):
 
    tc.desc = '''
    Test        :   SWM QoS Config test
    Opcode      :   Verify
    '''

    #get node names
    naples_nodes = api.GetNaplesNodes()
    if len(naples_nodes) == 0:
        api.Logger.error("No naples node found")
        return api.types.status.ERROR
    tc.node_name = naples_nodes[0].Name()

    tc.cmd_cookies = []

    # setup ncsi
    naples_nodes = api.GetNaplesNodes()
    if len(naples_nodes) == 0:
        api.Logger.error("No naples node found")
        return api.types.status.ERROR

    test_node = naples_nodes[0]
    cimc_info = test_node.GetCimcInfo()

    if not cimc_info:
        api.Logger.error("CimcInfo is None, exiting")
        return api.types.status.ERROR
    try:
        check_set_ncsi(cimc_info)
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.ERROR

    return api.types.status.SUCCESS

def Trigger(tc):

    #==============================================================
    # trigger the commands
    #==============================================================

    result = api.types.status.SUCCESS

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    result = TriggerSWMQoSConfigVerfication(req, tc)

    return result

def Verify(tc):

    #==============================================================
    # verify the output
    #==============================================================

    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS

    result = VerifySWMQoSConfigVerfication(tc)

    return result

def Teardown(tc):

    #==============================================================
    # cleanup
    #==============================================================

    return api.types.status.SUCCESS

