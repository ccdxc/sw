#! /usr/bin/python3
import copy
import pdb

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
        self.port   = spec.port.get()
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
        self.curr    = None
        self.prev   = None
        self.slist      = [ self.initial ]
        return

    def Init(self, spec, lg):
        self.lg = lg
        self.initial.Init(spec, lg)
        self.curr = copy.copy(self.initial)
        return

    def Get(self):
        return self.curr

    def GetPort(self):
        return self.curr.port

    def Advance(self):
        self.prev = self.curr
        self.slist.append(self.prev)
        self.curr = copy.copy(self.curr)
        return

    def IncrementSeqNum(self, size):
        self.curr.seq += size
        return

    def IncrementAckNum(self, size):
        self.curr.ack += size
        return

    def ShowDiff(self, lg):
        if self.prev.seq != self.curr.seq:
            lg.info("- Seq  : prev=", self.prev.seq, " curr=", self.curr.seq)
        if self.prev.ack != self.curr.ack:
            lg.info("- Ack  : prev=", self.prev.ack, " curr=", self.curr.ack)
        if self.prev.win != self.curr.win:
            lg.info("- Win  : prev=", self.prev.win, " curr=", self.curr.win)
        if self.prev.scale != self.curr.scale:
            lg.info("- Scale: prev=", self.prev.scale, " curr=", self.curr.scale)
        if self.prev.mss != self.curr.mss:
            lg.info("- MSS  : prev=", self.prev.mss, " curr=", self.prev.mss)
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
        self.lg.info("IFLOW: Advancing IFLOW by SeqIncr:%d" % seqincr)
        self.iflowstate.IncrementSeqNum(seqincr)
        self.lg.info("IFLOW: Advancing RFLOW by AckIncr:%d" % ackincr)
        self.rflowstate.IncrementAckNum(ackincr)
        return

    def __advance_rflow(self, seqincr, ackincr):
        self.lg.info("RFLOW: Advancing RFLOW by SeqIncr:%d" % seqincr)
        self.rflowstate.IncrementSeqNum(seqincr)
        self.lg.info("RFLOW: Advancing IFLOW by AckIncr:%d" % ackincr)
        self.iflowstate.IncrementAckNum(ackincr)
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
        self.rflowstate.Advance()
        self.iflowstate.Advance()
        if step.IsIflow():
            self.__advance_iflow(seqincr, ackincr)
        else:
            self.__advance_rflow(seqincr, ackincr)

        self.lg.info("IFLOW FlowState Diff:")
        self.iflowstate.ShowDiff(self.lg)
        self.lg.info("RFLOW FlowState Diff:")
        self.rflowstate.ShowDiff(self.lg)

        return

    def GetState(self, iflow):
        if iflow:
            return self.iflowstate.Get()
        return self.rflowstate.Get()

    def GetInitiatorPort(self):
        return self.iflowstate.GetPort()

    def GetResponderPort(self):
        return self.iflowstate.GetPort()
