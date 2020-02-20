from scapy.all import sr1, IP, ICMP
import os
import iota.harness.api as api

def ping_v2(dst, cnt):
    PING_TIMEOUT = 3
    for _i in range(cnt): 
        reply = sr1(IP(dst=dst)/ICMP(), timeout=PING_TIMEOUT)
        if not reply:
            print("Attempt: %d, IP: %s Unsuccessfull" % (_i, dst))
            continue

        if int(reply.getlayer(ICMP).type) == 0 \
                and int(reply.getlayer(ICMP).code) == 0:
            print(dst, ': Host is responding to ICMP Echo Requests.')
            break
    else:
        return False
    return True


def ping(dst, max_pings=1, timeout=2):
    for _i in range(max_pings):
        api.Logger.debug("Ping attempt %d: ip %s" % (_i, dst))
        resp = os.system("ping -c 1 -W %d %s" % (timeout, dst))
        if resp == 0:
            api.Logger.info("Ping successful for ip %s" % (dst))
            return api.types.status.SUCCESS
    return api.types.status.FAILURE