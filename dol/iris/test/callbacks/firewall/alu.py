#! /usr/bin/python3

def Add(step, orig, args):
    return orig + args.val

def Sub(step, orig, args):
    return orig - args.val

def Mul(step, orig, args):
    return orig * args.val

def Div(step, orig, args):
    return orig * args.val

def Mod(step, orig, args):
    return orig % args.val

def AddWithWindow(step, orig, args):
    return orig + step.prfstate.window + args.val

def SubWithWindow(step, orig, args):
    return orig + step.prfstate.window - args.val
