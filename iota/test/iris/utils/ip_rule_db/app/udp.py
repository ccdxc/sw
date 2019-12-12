HINT_MATCH_SP = 1
HINT_MATCH_DP = 2

import logging

logger = logging.getLogger()

from util.port import *
class Udp:
    def __init__(self, **kwargs):
        self.hint = 0

        self.sp = kwargs.get("sp", "any")
        self.dp = kwargs.get("dp", "any")

    @property
    def sp(self):
        return self._sp

    @sp.setter
    def sp(self, param):
        try:
            value = Port(param)
            if value.portType == PORT_TYPE.PORT_ANY:
                self.hint &= ~HINT_MATCH_SP
            else:
                self.hint |= HINT_MATCH_SP

            self._sp = value
        except:
            raise ValueError("Invalid src port %s"%param)

    @property
    def dp(self):
        return self._dp

    @dp.setter
    def dp(self, param):
        try:
            value = Port(param)
            if value.portType == PORT_TYPE.PORT_ANY:
                self.hint &= ~HINT_MATCH_DP
            else:
                self.hint |= HINT_MATCH_DP

            self._dp = value
        except:
            raise ValueError("Invalid dst port %s"%param)

    def __str__(self):
        return "sp=%s, dp=%s"%(self.sp, self.dp)

    def __eq__(self, obj):
        return isinstance(obj, Udp) and \
            self.sp == obj.sp and \
            self.dp == obj.dp

    def match(self, **kwargs):
        sp = kwargs.get('sp', None)
        dp = kwargs.get('dp', None)

        if not isinstance(sp,int):
            logger.warning("While matching UDP, source port field 'sp' is must")
            return False

        if not isinstance(dp,int):
            logger.warning("While matching UDP, destination port field 'dp' is must")
            return False

        if self.hint & HINT_MATCH_SP and \
           sp not in self.sp:
            return False

        if self.hint & HINT_MATCH_DP and \
           dp not in self.dp:
            return False

        return True

    def applyFilter(self, **kwargs):
        for k,v in kwargs.items():
            if k == "sp" and v not in self.sp:
                return False

            if k == "dp" and v not in self.dp:
                return False
        return True


    def getRandom(self, seed=None):
        return {
            "sp":self.sp.getRandom(seed=seed),
            "dp":self.dp.getRandom(seed=seed)
        }
