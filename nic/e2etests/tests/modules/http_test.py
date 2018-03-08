import pdb
http_server = None

def SetUp(src, dst):
    pass
    
def Run(src, dst):
	global http_server
	http_server = src.Run("python -m SimpleHTTPServer 8000", background=True)
	if not http_server:
		return False
	return dst.Run("wget %s:8000" % src.GetIp())
    
def Teardown(src, dst):
    http_server.kill()
