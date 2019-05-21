#!/usr/bin/python
import os
import time

os.system("rm -rf /data/savedlogs/ && mkdir /data/savedlogs/")
srcfile='/var/log/pensando/pds-agent.log.1'
lognum = 1
while True:
   if os.path.isfile(srcfile):
       dstfile = '/data/savedlogs/pds-agent.log.%d' % lognum
       os.system("mv %s %s" % (srcfile, dstfile))
       os.system("gzip %s" % dstfile)
       print("Saved %s" % dstfile)
       lognum += 1
   else:
       time.sleep(1)
