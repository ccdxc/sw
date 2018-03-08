import pdb
import os
ftp_server = None

def SetUp(src, dst):
    pass
    
def Run(src, dst):
    global ftp_server
    ftp_server = src.Run("python3 -m pyftpdlib -i %s -p 21 -u admin -P root" % src.GetIp(), background=True)
    if not ftp_server:
        return False
    return dst.Run("wget -m ftp://admin:root@%s/e2etests/tests/files/ftp_file.txt" % src.GetIp())
    
def Teardown(src, dst):
    os.system("rm -f ftp_file.txt") 
    ftp_server.kill()
