# /usr/bin/python3
import pdb
import apollo.config.utils as utils

def __get_mirror_objects(vnic, direction):
    return vnic.RxMirrorObjs if direction == 'rx' else vnic.TxMirrorObjs

def GetRSPANPortID(testcase, args):
    vnic = testcase.config.localmapping.VNIC
    objs = __get_mirror_objects(vnic, args.direction)
    mirrorObj = objs.get(args.id, None)
    return utils.GetPortIDfromInterface(mirrorObj.Interface) if mirrorObj else None

def GetRSPANVlanID(testcase, packet, args=None):
    vnic = testcase.config.localmapping.VNIC
    objs = __get_mirror_objects(vnic, args.direction)
    mirrorObj = objs.get(args.id, None)
    return mirrorObj.VlanId if mirrorObj else 0

def GetExpectedPacket(testcase, args):
    vnic = testcase.config.localmapping.VNIC
    mirrorid = args.id
    objs = __get_mirror_objects(vnic, args.direction)
    mirrorObj = objs.get(mirrorid, None)
    return testcase.packets.Get('MIRROR_PKT_' + str(mirrorid)) if mirrorObj else None
