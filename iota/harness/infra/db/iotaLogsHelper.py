import pdb
import re
import os
import subprocess
import time
import traceback

def getIotaDetailedLog(jobdId,dest,hostlog):
    if dest in ["","/",None]:
        raise ValueError("must specify non root and non empty dest directory")
    if not dest.endswith("/"): dest += '/'
    dest += "{0}/".format(jobdId)
    try:
        subprocess.check_call("jobc savedlog {0} -o {1}".format(jobdId,dest),
                                   stdout=subprocess.PIPE, shell=True)
        subprocess.check_call("tar -zxvf {0}iota_sanity_logs.tar.gz -C {0} {1}".format(dest,hostlog),
                              stdout=subprocess.PIPE, shell=True)
    finally:
        print("deleting log dir {0}".format(dest))
        try: os.remove(dest)
        except: pass
    return dest+hostlog

def isFirmwareFailure(text):
    ffhRe = re.compile('FIRMWARE UPGRADE / MODE CHANGE / REBOOT FAILED: LOGFILE = ([\S]+)[-]+')
    found = ffhRe.search(text)
    if found:
        return found.group(1)

def getFailedHostText(jobdId,dest,hostlog):
    logfile = getIotaDetailedLog(jobdId,dest,hostlog)
    text = getEndOfLog(logfile, 10)
    print('removing hostlog {0}'.format(logfile))
    try: os.remove(logfile)
    except: pass
    return text

def getEndOfLog(logfile, lc=500):
    return subprocess.check_output("tail -n {0} {1}".format(lc, logfile), shell=True, stderr=subprocess.STDOUT).decode("utf-8")

def getJobdConsoleLog(jobdId,dest):
    if dest in ["",None]:
        raise ValueError("must specify non empty dest directory")
    if not dest.endswith("/"): dest += '/'
    zname = "{0}.gzip".format(jobdId)
    fpath = "{0}{1}".format(dest,zname)
    cmd = "curl http://jobd:3456/logs/full/{0} -s --output {1}".format(jobdId,fpath)
    print("getting jobd console log with command: {0}".format(cmd))
    subprocess.check_call(cmd.split(" "), stdout=subprocess.PIPE)
    try:
        subprocess.check_call("gunzip -f -S gzip {0}".format(fpath), 
                               stdout=subprocess.PIPE, shell=True)
    except:
        try: os.remove(fpath)
        except: pass
        raise Exception("failed to unzip logfile {0}. error was: {1}".format(fpath, traceback.format_exc()))
    fname = "{0}{1}.log".format(dest,jobdId)
    os.rename("{0}{1}.".format(dest,jobdId),fname)
    print('downloaded console logfile {0}'.format(fname))
    return fname

def cleanupLogs(logfile):
    print('deleting console logfile {0}'.format(logfile))
    try: os.remove(logfile)
    except: pass

def getTsStartTimes(logfile):
    #not in logfile yet, only time, not date.
    return {}
    with open(logfile, 'r') as infile:
        entries = {}
        stsRe = re.compile("\[([\S]+)\] \[TS:([\w]+)\]\[INFO\] Starting Testsuite: ([\w]+)")
        for line in infile:
            found = stsRe.search(line)
            if found:
                entries[found.group(2)] = found.group(1)
        return entries

def getTsVersions(logfile):
    #not in logfile yet
    return {}

def findTsSummary(lines):
    tsr = re.split("Testsuite[\s]+Pass[\s]+Fail[\s]+Ignore[\s]+Dis/Err[\s]+Total[\s]+Target[\s]+%Done[\s]+Result[\s]+Duration\r\n[-]+\r\n",lines,1)[1]
    tsr =  re.split('[-]+\r\n',tsr,1)[0]
    return tsr

def buildEntriesFromTsSummary(text,logfile,jobdId):
    startTimes = getTsStartTimes(logfile)
    versions = getTsVersions(logfile)
    tsr = ""
    entries = []
    tsnRe = re.compile("^([\S]+)")
    for line in text.split('\r\n'):
        if len(line)<5:
            continue
        tsName = tsnRe.search(line).group(1)
        entry = {'text':line, 'version':versions.get(tsName,'NA'), 'startTime':startTimes.get(tsName,time.asctime()), 'jobdId':jobdId, 'data':'NA'}
        entries.append(entry)
    return entries

def getTsResultsEntries(logfile, jobdId):
    lines = getEndOfLog(logfile)
    try:
        text = findTsSummary(lines)
    except:
        print("failed to find test suite summary in console output for job {0}".format(jobdId))
        return []
    return buildEntriesFromTsSummary(text, logfile, jobdId)
        
def parseTsResultsText(jobdId,dest):
    logfile = None
    try:
        print('proc jobdId:{0}'.format(jobdId))
        logfile = getJobdConsoleLog(jobdId,dest)
        results = getTsResultsEntries(logfile, jobdId)
    except:
        print("failed to process job {0}. error was: {1}".format(jobdId, traceback.format_exc()))
        return []
    finally:
        if logfile:
            cleanupLogs(logfile)
    return results

