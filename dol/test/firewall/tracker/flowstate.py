#! /usr/bin/python3
import copy
import pdb

class FlowStateData:
    def __init__(self):
        self.lg     = None
        self.seq    = None
        self.ack    = None
        self.flags  = None
        self.window    = None
        self.scale  = None
        self.mss    = None
        return

    def Init(self, spec, lg):
        self.port   = spec.port.get()
        self.seq    = getattr(spec, 'seq', self.seq)
        self.ack    = getattr(spec, 'ack', self.ack)
        self.flags  = getattr(spec, 'flags', self.flags)
        self.window = getattr(spec, 'window', self.window)
        self.scale  = getattr(spec, 'scale', self.scale)
        self.mss    = getattr(spec, 'mss', self.mss)
        return

    def __copy__(self):
        obj = type(self)()
        obj.__dict__.update(self.__dict__)
        return obj

    def IncrementSeqNum(self, size):
        self.seq += size
        self.seq &= 0xFFFFFFFF
        return
    def IncrementAckNum(self, size):
        self.ack += size
        self.ack &= 0xFFFFFFFF
        return
    def SetSeq(self, seq):
        self.seq = seq
        self.seq &= 0xFFFFFFFF
        return
    def SetAck(self, ack):
        self.ack = ack
        self.ack &= 0xFFFFFFFF
        return
    def SetWindow(self, win):
        self.window = win
        return
    def SetScale(self, scale):
        self.scale = scale
        return
    def SetMss(self, mss):
        self.mss = mss
        return

    def Show(self, lg):
        lg.info("- Seq  : ", self.seq)
        lg.info("- Ack  : ", self.ack)
        lg.info("- Flags: ", self.flags)
        lg.info("- Win  : ", self.window)
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
        self.curr.IncrementSeqNum(size)
        return

    def IncrementAckNum(self, size):
        self.curr.IncrementAckNum(size)
        return

    def SetSeq(self, seq):
        self.curr.SetSeq(seq)
        return
    def SetAck(self, ack):
        self.curr.SetAck(ack)
        return
    def SetWindow(self, win):
        self.curr.SetWindow(win)
        return
    def SetScale(self, scale):
        self.curr.SetScale(scale)
        return
    def SetMss(self, mss):
        self.curr.SetMss(mss)
        return


    def ShowDiff(self, lg):
        if self.prev.seq != self.curr.seq:
            lg.info("- Seq  : prev =", self.prev.seq, " curr =", self.curr.seq)
        if self.prev.ack != self.curr.ack:
            lg.info("- Ack  : prev =", self.prev.ack, " curr =", self.curr.ack)
        if self.prev.window != self.curr.window:
            lg.info("- Win  : prev =", self.prev.window, " curr =", self.curr.window)
        if self.prev.scale != self.curr.scale:
            lg.info("- Scale: prev =", self.prev.scale, " curr =", self.curr.scale)
        if self.prev.mss != self.curr.mss:
            lg.info("- MSS  : prev =", self.prev.mss, " curr =", self.prev.mss)
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

    def __advance_common_abs(self, prefix, flow, flowstate):
        seq = getattr(flow, 'seq', None)
        if seq is not None:
            self.lg.info("%s: ABS Advancing Seq:%d" % (prefix, seq))
            flowstate.SetSeq(seq)

        ack = getattr(flow, 'ack', None)
        if seq is not None:
            self.lg.info("%s: ABS Advancing Ack:%d" % (prefix, ack))
            flowstate.SetAck(seq)

        win = getattr(flow, 'window', None)
        if win is not None:
            self.lg.info("%s: ABS Advancing Win:%d" % (prefix, win))
            flowstate.SetWindow(win)

        scale = getattr(flow, 'scale', None)
        if scale is not None:
            self.lg.info("%s: ABS Advancing Scale:%d" % (prefix, scale))
            flowstate.SetScale(scale)
           
        mss = getattr(flow, 'mss', None)
        if mss is not None:
            self.lg.info("%s: ABS Advancing MSS:%d" % (prefix, mss))
            flowstate.SetMss(mss)
        return

    def __advance_iflow_abs(self, flow):
        self.__advance_common_abs("IFLOW", flow, self.iflowstate)
        return
 
    def __advance_rflow_abs(self, flow):
        self.__advance_common_abs("RFLOW", flow, self.rflowstate)
        return

    def __advance(self, step):
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
        return

    def __advance_abs(self, step):
        iflow = getattr(step.state, 'iflow', None)
        if iflow is not None:
            self.__advance_iflow_abs(iflow)

        rflow = getattr(step.state, 'rflow', None)
        if rflow is not None:
            self.__advance_rflow_abs(rflow)
        return

    def Advance(self, step):
        state = getattr(step, 'state', None)
        if state is None:
            self.__advance(step)
        else:
            self.__advance_abs(step)

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
