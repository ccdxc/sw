#! /usr/bin/python3

import pdb
import infra.config.base as base
import infra.common.objects as objects
from infra.common.logging import logger
from config.store import Store

import model_sim.src.model_wrap as model_wrap


class Timer(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        return

    def Init(self, spec):
        self.GID(spec.id)
        self.id = spec.id
        self.slowfast = int(spec.slowfast)
        self.ctime = int(spec.ctime)

    def Step(self, ctime, lgh=logger):
        lgh.info("Setting Timer %s: ctime %d" % (self.id, ctime))
        model_wrap.step_tmr_wheel_update(self.slowfast, ctime)

class TimerObjectHelper:
    def __init__(self):
        self.objlist = []
        return

    def Generate(self, topospec):
        timerspec = getattr(topospec, 'timers', None)
        if timerspec == None: return
        for timer in topospec.timers:
            obj = Timer()
            obj.Init(timer.entry)
            self.objlist.append(obj)
        Store.objects.SetAll(self.objlist)

    def main(self, topospec):
        self.Generate(topospec)
        logger.info("Adding %d Timers to Store." % len(self.objlist))

TimerHelper = TimerObjectHelper()
