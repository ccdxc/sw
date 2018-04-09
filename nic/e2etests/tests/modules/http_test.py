import pdb
http_server = None

def SetUp(src, dst):
    pass
    
def Run(src, dst):
    global http_server
    http_server = src.Run("python -m SimpleHTTPServer 8000", background=True)
    if http_server:
        return False
    if dst.Run("wget %s:8000" % src.GetIp()):
        return False
    return True
    
def Teardown(src, dst):
    http_server.kill()
