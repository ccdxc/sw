from collections import OrderedDict as od
from functools import wraps
import datetime
import logging
import pdb
import psycopg2
import re
import sys
import time
import traceback

logger = logging.getLogger()

class PgHelper(object):

    _testSuiteHeaders=['name','pass','fail','ignore','error','total','target','done','result','duration','jobdId','version','startTime','data']

    def __init__(self, dbHost, dbName, username, password):
        self.dbHost = dbHost
        self.dbName = dbName
        self.username = username
        self.password = password
        self.conn = None
        self.cur = None
        self.durRe = re.compile('(\d\d):(\d\d):(\d\d)\.[\d]+')

    def getHdrs(self):
        return PgHelper._testSuiteHeaders

    def connect(func):
        @wraps(func)
        def conWrap(*args,**kwargs):
            try:
                self = args[0]
                self.conn = psycopg2.connect(host=self.dbHost, dbname=self.dbName,
                                             user=self.username, password=self.password)
                self.cur = self.conn.cursor()
                return func(*args,**kwargs)
            except:
                msg = "failed call to {0}. error was: {1}".format(func.__name__,
                      traceback.format_exc())
                print(msg)
                logger.error(msg)
            finally:
                try: self.cur.close()
                except: pass
                try: self.conn.close()
                except: pass
        return conWrap;

    def ignoreError(func):
        @wraps(func)
        def ignWrap(*args,**kwargs):
            try:
                return func(*args,**kwargs)
            except:
                msg = "ignoring error {0}".format(sys.exc_info()[0])
                print(msg)
                logger.error(msg)
        return ignWrap;

    def commit(func):
        @wraps(func)
        def comWrap(*args,**kwargs):
            try:
                func(*args,**kwargs)
                self = args[0]
                self.conn.commit()
            except:
                msg = "failed to call commit. error was: {0}".format(traceback.format_exc())
                print(msg)
                logger.error(msg)
        return comWrap;

    def execute(func):
        @wraps(func)
        def exeWrap(*args,**kwargs):
            try:
                cmd = func(*args,**kwargs)
                self = args[0]
                self.cur.execute(cmd)
            except:
                msg = "failed to call execute. error was: {0}".format(traceback.format_exc())
                print(msg)
                logger.error(msg)
        return exeWrap;

    @connect
    @commit
    @ignoreError
    def createTestSuiteResultsTable(self):
        self.cur.execute("CREATE TABLE test_suite_results (id serial PRIMARY KEY, name varchar, pass integer, fail integer, ignore integer, error integer, total integer, target integer, done varchar, result varchar, duration integer, jobdId integer, version varchar, startTime timestamp, data varchar);")

    @connect
    @commit
    @ignoreError
    def createTestBundleResultsTable(self):
        self.cur.execute("CREATE TABLE test_bundle_results (id serial PRIMARY KEY, name varchar, startTime timestamp, endTime timestamp, version varchar, result varchar, data varchar);")

    @connect
    @commit
    @ignoreError
    def createHwBringupResultsTable(self):
        self.cur.execute("CREATE TABLE hardware_bringup_results (id serial PRIMARY KEY, name varchar, startTime timestamp, endTime timestamp, version varchar, result varchar, data varchar);")

    def createTables(self):
        self.createTestSuiteResultsTable()
        self.createTestBundleResultsTable()
        self.createHwBringupResultsTable()

    def formatTimestamps(self, startTime, endTime):
        if type(startTime) == datetime.datetime:
            startTime = self.cur.mogrify("{0}".format(startTime))
        if type(endTime) == datetime.datetime:
            endTime = self.cur.mogrify("{0}".format(endTime))
        elif not endTime:
            endTime = ""
        return startTime, endTime

    @connect
    @commit
    @execute
    def addTestSuiteResults(self, sp):
        lmsg = "INSERT INTO test_suite_results ("
        rmsg = ") VALUES ("
        for hdr in self.getHdrs():
            lmsg += "{0}, ".format(hdr)
            if type(sp[hdr]) == str:
                rmsg += "'{0}', ".format(sp[hdr])
            else:
                rmsg += "{0}, ".format(sp[hdr])
        lmsg = lmsg.rstrip(', ')
        rmsg = rmsg.rstrip(', ') + ")"
        msg = lmsg + rmsg
        cmd = self.cur.mogrify(msg)
        return cmd

    def buildTsEntry(self, line, jobdId, version, data='', startTime=time.asctime()):
        data = zip(self.getHdrs(),re.split('[\s]+',line) + [jobdId, version, startTime, data])
        entry = od(data)
        td = self.durRe.search(entry['duration'])
        if td:
            entry['duration'] = str(int(td.group(1))*3600+int(td.group(2))*60+int(td.group(3)))
        else:
            raise ValueError('expecting duration to be format 00:00:00.000, but found: {0}'.format(entry['duration']))
        return entry

    def updateTsResultsDb(self, testResults):
        entries = []
        for res in testResults:
            entry = self.buildTsEntry(res['text'], jobdId=res['jobdId'], version=res['version'], startTime=res['startTime'], data=res['data'])
            self.addTestSuiteResults(entry)

