import pdb

def SetUp(src, dst):
    pass
    
def Run(src, dst):
    ret = src.Run("ping -c 5  -d -i 3 " + dst.GetIp())
    if ret:
        return False
    if dst.Run("ping -c 5  -d -i 3 " + src.GetIp()):
        return False
    return True
    
def Teardown(src, dst):
    pass