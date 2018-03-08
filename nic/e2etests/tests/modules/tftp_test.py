import pdb
import os
tftp_server = None

def SetUp(src, dst):
    pass
    
def Run(src, dst):
    global tftp_server
    tftp_server = src.Run("python /bin/tftpy_server.py -i %s -r /sw/nic" % src.GetIp(), background=True)
    if not tftp_server:
        return False
    return dst.Run("python /bin/tftpy_client.py -H %s -D /sw/nic/e2etests/tests/files/tftp_file.txt" % src.GetIp())
    
def Teardown(src, dst):
    os.system("rm -f tftp_file.txt")
    tftp_server.kill()
