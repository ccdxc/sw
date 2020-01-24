#!/usr/bin/python
import re
import os
from optparse import OptionParser
import sys

libs = []

def findInSubdirectory(filename, subdirectory=''):
    if subdirectory:
        path = subdirectory
    else:
        path = os.getcwd()
    for root, dirs, names in os.walk(path):
        if filename in names:
            return os.path.join(root, filename)
    #print 'File not found'
    return None

def main(argv):

    parser = OptionParser()

    # Get dir path
    parser.add_option("-s", "--subdirectory", dest="subdirectory", type="string",
            help="Specifies path to logfile", metavar="subdirectory")
    parser.add_option("-f", "--deps-file", dest="deps_file", type="string",
            help="Specifies path to deps_file", metavar="deps_file")


    (options, args) = parser.parse_args()

    subdirectory = options.subdirectory
    dependency_file = options.deps_file

    d_file = open(dependency_file, "r")
    for line in d_file:
        #print line
        line = line.strip()
        line = line.rstrip()
        #new_line = re.sub("\[", "", line)
        new_line = re.sub('[\[\]]', "", line)
        #print new_line
        libs.append(new_line)

    #print libs
    missing_libs = []

    for lib in libs:
        path_to_file = findInSubdirectory(lib, subdirectory)

        if path_to_file != None:
            #print 'File found at location: %s' % path_to_file
            pass
        else:
            #print '%s library not found' % lib
            missing_libs.append(lib)

    print missing_libs

if __name__ == "__main__":
        main(sys.argv)

