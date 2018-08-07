#! /usr/bin/python3
import copy
import pdb

def __repl(srcobj, count, deep = True):
    objs = []
    for i in range(int(count)):
        if deep:
            obj = copy.deepcopy(srcobj)
        else:
            obj = copy.copy(srcobj)
        new_gid = obj.GID() + "_%d" % (i+1)
        obj.GID(new_gid)
        objs.append(obj)
    return objs


def ReplicatePackets(tc, args):
    srcobj = tc.packets.Get(args.src)
    return __repl(srcobj, args.count)

def ReplicateDescriptors(tc, args):
    srcobj = tc.descriptors.Get(args.src)
    return __repl(srcobj, args.count, deep = False)

def ReplicateBuffers(tc, args):
    srcobj = tc.buffers.Get(args.src)
    return __repl(srcobj, args.count, deep = False)
