#!/usr/bin/python
import os
import time

destdir='/tmp/savedlogs'
srcfile='/sw/nic/pds-agent.log.1'
os.system("rm -rf " + destdir + " && mkdir " + destdir)
lognum = 1
while True:
   if os.path.isfile(srcfile):
       dstfile = destdir + '/pds-agent.log.%d' % lognum
       os.system("mv %s %s" % (srcfile, dstfile))
       os.system("gzip %s" % dstfile)
       print("Saved %s" % dstfile)
       lognum += 1
   else:
       time.sleep(1)
