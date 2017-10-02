#! /usr/bin/python3
import copy

class FlowStateData:
    def __init__(self):
        self.lg     = None
        self.seq    = None
        self.ack    = None
        self.flags  = None
        self.win    = None
        self.scale  = None
        self.mss    = None
        return

    def Init(self, spec, lg):
        self.seq    = getattr(spec, 'seq', self.seq)
        self.ack    = getattr(spec, 'ack', self.ack)
        self.flags  = getattr(spec, 'flags', self.flags)
        self.win    = getattr(spec, 'win', self.win)
        self.scale  = getattr(spec, 'scale', self.scale)
        self.mss    = getattr(spec, 'mss', self.mss)
        return

    def __copy__(self):
        obj = type(self)()
        obj.__dict__.update(self.__dict__)
        return obj

    def IncrSeqNum(self, size):
        self.seq += size
        lg.info("Advancing SEQ number to %d" % self.seq)
        return

    def Show(self, lg):
        lg.info("- Seq  : ", self.seq)
        lg.info("- Ack  : ", self.ack)
        lg.info("- Flags: ", self.flags)
        lg.info("- Win  : ", self.win)
        lg.info("- Scale: ", self.scale)
        lg.info("- MSS  : ", self.mss)
        return


class FlowState:
    def __init__(self):
        self.initial    = FlowStateData()
        self.current    = None
        self.previous   = None
        self.slist      = [ self.initial ]
        return

    def Init(self, spec, lg):
        self.lg = lg
        self.initial.Init(spec, lg)
        self.current = copy.copy(self.initial)
        return

    def Get(self):
        return self.current

    def Advance(self):
        self.previous = self.current
        self.slist.append(self.previous)
        self.current = copy.copy(self.current)
        return

    def IncrementSeqNum(self, size):
        self.current.seq += size
        return

    def IncrementAckNum(self, size):
        self.current.ack += size
        return

    def Show(self, lg):
        self.current.Show(lg)
        return

class FlowStateTracker:
    def __init__(self):
        self.lg         = None
        self.iflowstate = FlowState()
        self.rflowstate = FlowState()
        return

    def Init(self, connspec, lg):
        self.lg = lg
        self.iflowstate.Init(connspec.initiator, lg)
        self.rflowstate.Init(connspec.responder, lg)
        return

    def __advance_iflow(self, seqincr, ackincr):
        self.iflowstate.IncrementSeqNum(seqincr)
        self.rflowstate.IncrementAckNum(ackincr)
        self.lg.info("Advancing IFLOW by SeqIncr:%d AckIncr:%d" % (seqincr, ackincr))
        return

    def __advance_rflow(self, seqincr, ackincr):
        self.rflowstate.IncrementSeqNum(seqincr)
        self.iflowstate.IncrementAckNum(ackincr)
        self.lg.info("Advancing RFLOW by SeqIncr:%d AckIncr:%d" % (seqincr, ackincr))
        return

    def Advance(self, step):
        incr = 0
        if step.IsSyn() or step.IsFin():
            incr = 1
        incr += step.payloadsize
        seqincr = incr
        ackincr = incr
        if step.IsSyn():
            ackincr += step.fields.seq

        self.lg.info("Advancing Tracker after Step:%s Flags:%s" %\
                     (step.GID(), step.fields.flags))
        if step.IsIflow():
            self.__advance_iflow(seqincr, ackincr)
        else:
            self.__advance_rflow(seqincr, ackincr)

        self.lg.info("IFLOW FlowState Data:")
        self.iflowstate.Show(self.lg)
        self.lg.info("RFLOW FlowState Data:")
        self.rflowstate.Show(self.lg)

        return

    def GetState(self, iflow):
        if iflow:
            return self.iflowstate.Get()
        return self.rflowstate.Get()
