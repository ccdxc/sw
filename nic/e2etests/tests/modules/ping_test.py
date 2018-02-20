import pdb

def SetUp(infra):
    pass
    
def Run(src, dst):
    ret = src.Run("ping -c 5  -d -i 3 " + dst.GetIp())
    if not ret:
        return ret
    return dst.Run("ping -c 5  -d -i 3 " + src.GetIp())
    
def Teardown(infra):
    pass