#!/usr/bin/python
from collections import defaultdict
import re
from os import path, listdir, getcwd, chdir

class Graph (object):
    def __init__ (self):
        """
        
        """
        self.meta = defaultdict(lambda: [-1, False]) 
        self.vectors = list()
        pass

    def reinit (self):
        self.__init__()
 
    def isVisited (self, vec):
        return self.meta[vec][1]

    def setVisited (self, vec):
        self.meta[vec][1] = True

    def resetVisited (self, vec=None):
        if (vec == None):
            for k, v in self.meta.iteritems():
                 v[1] = False
        else:
            self.meta[vec][1] = False

    def getVecIdx (self, vec):
        return self.meta[vec][0]

    def allocSetVecIdx (self, vec):
        idx = len(self.vectors)
        self.meta[vec][0] = idx
        self.vectors.append(dict())
        self.vectors[-1]["name"] = vec
        self.vectors[-1]["edges"] = list()
        return idx


    def add2db (self, vec, parent):
        """
        Given an vector 
            1) insert into self.vectors
            2) get a list of all the children
            3) populate self.vectors[vec]
        """
        pIdx = self.getVecIdx(parent)
        idx = self.getVecIdx(vec)
        if idx == -1 :
             idx = self.allocSetVecIdx(vec)
        self.vectors[pIdx]["edges"].append(idx)                 

    def BFS (self, root, visitfn):
        if (self.isVisited(root) == True):
            return
        queue = list()
        queue.append(root)
        self.setVisited(root)
        self.allocSetVecIdx(root)
        while (len(queue) != 0):
            vec = queue.pop(0)
            cVecs = visitfn(vec)
            for entry in cVecs:
                if (self.isVisited(entry) != True):
                    self.add2db(entry, vec)
                    queue.append(entry)   
                    self.setVisited(entry) 

    def getConnectedVec (self, root_list):
        """
        Return a list of all Vectors rechable from root.
        """
        retList = list()
        self.resetVisited()
        for root in root_list:
            if (self.isVisited(root) == True):
                continue
            queue = list()
            queue.append(root)
            self.setVisited(root)
            while (len(queue) != 0):
                vec = queue.pop(0)
                retList.append(vec)
                cVecList = self.vectors[self.getVecIdx(vec)]["edges"]
                for idx in cVecList:
                    entry = self.vectors[idx]["name"]
                    if (self.isVisited(entry) != True):
                        queue.append(entry)   
                        self.setVisited(entry) 
        return retList

           
class Parser ( ):
    def __init__(self, prefixes=None, rwd=None, arch=None, asic="capri"):
        """
        rwd : Relative working directory from which 
              the script needs to run.         
        prefixs : list of prefix paths that can be
                  added to a file location to get
                  complete path of file from rwd.
        """
        self.rwd = rwd if rwd is not None else os.getcwd()
        if (prefixes == None):
            self.prefixes = ["nic/sdk",
                             "nic/sdk/dpdk/build/include",
                             "nic/hal/third-party/spdlog/include",
                             "nic/sdk/third-party/googletest-release-1.8.0/googletest/include",
                             "nic/sdk/third-party/boost/include",
                             "nic/sdk/model_sim/include"]

            if (arch != None):
                self.prefixes.append("nic/build/{0}/athena/{1}".format(arch, asic))
                self.prefixes.append("nic/build/{0}/athena/{1}/out/pen_dpdk_submake/include".format(arch, asic))
        else:
            self.prefixes = prefixes
        print "prefixes are {0}".format(self.prefixes) 
        self.updStdHeaders()
        self.db = Graph()
        self.failed = dict()
        chdir(rwd)

    def updStdHeaders (self):
        """
            Get a list of standard headers of C and C++
            Note: The location of header files are hardcoded for now.
                  This function looks only at current level and 
                  level down.
            Tested
        """

        hdrsLoc = ["/tool/toolchain/aarch64-1.1/aarch64-linux-gnu/libc/usr/include/",
                   "/tool/toolchain/aarch64-1.1/aarch64-linux-gnu/include/c++/6.4.1",
                   "/tool/toolchain/aarch64-1.1/lib/gcc/aarch64-linux-gnu/6.4.1/include"]
        self.stdHeaders = dict()
        for hdrLoc in hdrsLoc:
            for entry in listdir(hdrLoc):
                fpath = "{0}/{1}".format(hdrLoc, entry)
                if path.isdir(fpath):
                    for e in listdir(fpath):
                        if path.isfile("{0}/{1}".format(fpath, e)):
                            self.stdHeaders["{0}/{1}".format(entry,e)] = 1
                elif path.isfile(fpath):
                    self.stdHeaders[entry] = 1

    def parse (self, filePath):
        """
        Parse a file and return a list of header files
        relative to rwd
        1. If file dosent exist do nothing. else open the file.
        2. use regex expression and get the list of possible headerfiles
        3. check if header files exist. Else append the header files to
           possible prefix paths and check for the file.
        4. If file is not present then append it to fail list.
        5. Return both the lists          
        TODO: 
             a. Need to implement a good approach for Same header 
                name at multiple locations.
             b. If Same file name exists in standard libraries and 
                     locally when complete path is not given, it will pick
                     standard library. It should be other way
             c. Same header file defined in multiple locations, script will
                pick the first one encountered in prefixes list.
        """

        pat = '#\s*include\s+(?:(?:"\s*(.*\.h)\s*")|(?:"\s*(.*(?:\.hpp)?)\s*")|(?:<\s*(.*\.h)\s*>)|(?:<\s*(.*(?:\.hpp)?)\s*>))'
        regex = re.compile(pat)

        if False == path.exists(filePath) or False == path.isfile(filePath) :
            return [],[]

        with open(filePath, 'r') as fp:
            data = fp.read()
            headers = regex.findall(data) 

        #TODO: rewrite this code
        entries = list()
        failedEntries = list()
        passEntries = list()
        for entry in headers:
            for e in entry:
                 #assuming only one group out of four being match will be set.
                 if e != '':
                     entries.append(e)

        for entry in entries:
            found = False
            #print "Processing file {0}".format(entry)
            if False == path.exists(entry) or False == path.isfile(entry) :
                 try:
                     #        print pEntsprint "{0} in {1} self.stdHeaders".format(entry, True if self.stdHeaders[entry] == 1 else False)
                     self.stdHeaders[entry]
                 except KeyError:
                     #print "Key error for {0}".format(entry)
                     # check in the directory of parent file.
                     pDirEndIdx = filePath.rfind("/")
                     pDir = filePath[:pDirEndIdx]
                     tmpPath = "{0}/{1}".format(pDir, entry)
                     if (path.exists(tmpPath) and path.isfile(tmpPath)):
                         found = True
                         passEntries.append(tmpPath)
                     else :
                         for prefix in self.prefixes:
                             tmpPath = "{0}/{1}".format(prefix, entry)
                             if (path.exists(tmpPath) and path.isfile(tmpPath)):
                                 found = True
                                 passEntries.append(tmpPath)
                                 break; 
                     if found == False: 
                          failedEntries.append(entry)
            else :
                passEntries.append(entry)

        return entries, passEntries, failedEntries

    def retIncludes (self, fPath):
        """
        Returns all the non standard header files included
        by a file.
        """
        print "Entered retIncludes for file {0}".format(fPath)
        ents, pEnts, fEnts = self.parse(fPath)
        print ("file {0} has tot {1}, non Std {2}, fld {3} hdrs"
               .format(fPath, len(ents), len(pEnts), len(fEnts)))

        if (len(fEnts) > 0):
            self.failed[fPath] = fEnts
        print pEnts
        return pEnts

    def printSummary (self, detail=False):
       """
           print Summary info
            1. For each file in input file or directory print
                Filename , total recursive dependent headerfiles, 
                total standard headers, non-standard headers, 
                failed to resolve headers
        """

    def process (self, loc, dotC=True, incPar=False):    
        """
            Given a file or directory defined by loc
            calculate all the dependent header files
            loc : file or directory.
            dotC : If loc is directory, this flag tells
                   to consider either .*c[c] or .*h[pp] files.
            incPar: Include file[s] referenced by file/dir "loc".
        """
        if False == path.exists(loc):
            return 

        finFiles = dict()
        if path.isfile(loc):
            # add the directory of the file to prefixes as there
            # might be some .h/.hpp defined 
            idx = loc.rfind("/")
            if idx != -1:
                self.prefixes.insert(0, loc[0:idx])
            files = [loc]
        elif path.isdir(loc):
            def consider (f):
                print "consider file {0}".format("/".join([loc, f]))
                print "is path file {0}".format(path.isfile("/".join([loc, f])))
                if (path.isfile("/".join([loc, f])) and
                        ((dotC == True and 
                        (f.endswith(".c") or f.endswith(".cc"))) or
                        (dotC == False and
                        (f.endswith(".h") or f.endswith(".hpp"))))):
                    print "return True"
                    return True
                print "return False"
                return False 
            files = ["/".join([loc,f]) for f in listdir(loc) if True == consider(f)]
            self.prefixes.insert(0, loc)
        for entry in files:
            # Build a graph db.              
            self.db.BFS(entry, self.retIncludes)

        retFiles = self.db.getConnectedVec(files)
        print "Done"
        if (incPar == False):
            retSet = set(retFiles)
            print "rset is ", retSet
            print "infiles are", set(files)
            print "difference is ", retSet.difference(set(files))
            retSet.difference(set(files))
            return retSet
        return set(retFiles)

if __name__ == '__main__':
    parser = Parser(rwd="/local/murali/binaries/sw")
    hfiles = parser.process("nic/apollo/test/athena_app") 
    print "Total number of hfiles needed are {0}".format(len(hfiles))
    for e in hfiles:
        print e
    parser.db.reinit()
    hfiles = parser.process("nic/apollo/api/include/athena", False)
    print "Total number of hfiles needed are {0}".format(len(hfiles))
    for e in hfiles:
        print e
