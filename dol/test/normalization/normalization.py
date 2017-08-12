#! /usr/bin/python3

def config(fwdata):
    asp = fwdata.Store.Get('ACTIVE_SECURITY_PROFILE')
    args = fwdata.module_args
    asp.profile = args.profile
    asp.Configure()
    return
