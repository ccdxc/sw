#! /usr/bin/python3

import pdb
import glob
import os
import fnmatch

import infra.common.dyml as dyml
import infra.common.defs as defs
import infra.common.objects as objects
import infra.common.utils as utils

from infra.common.logging import logger

class ParserBase:
    def __init__(self):
        return

    def __parse_file(self, filename):
        logger.debug("Parsing file =", filename)
        return dyml.main(filename)

    def __process_list(self, elem_list):
        fobj = []
        for elem in elem_list:
            if isinstance(elem, objects.FrameworkObject):
                elemobj = self.__process_yml_obj(elem)
            elif isinstance(elem, list):
                logger.verbose("Processing LIST %s" % elem)
                elemobj = self.__process_list(elem)
            elif utils.IsTemplateField(elem):
                logger.verbose("Processing TEMPLATE FIELD %s" % elem)
                elemobj = objects.TemplateFieldObject(elem)
            else:
                elemobj = elem
            fobj.append(elemobj)
        return fobj

    def __process_yml_obj(self, yobj):
        obj = objects.FrameworkTemplateObject()
        for fname,fval in yobj.__dict__.items():
            if isinstance(fval, list):
                logger.verbose("Processing LIST %s" % fname)
                fobj = self.__process_list(fval)
            elif isinstance(fval, objects.FrameworkObject):
                logger.verbose("Processing OBJECT %s" % fname)
                fobj = self.__process_yml_obj(fval)
            elif utils.IsTemplateField(fval):
                logger.verbose("Processing TEMPLATE FIELD %s = %s" % (fname, fval))
                fobj = objects.TemplateFieldObject(fval)
            else:
                logger.verbose("Processing basic FIELD %s =" % fname, fval)
                fobj = fval
            obj.__dict__[fname] = fobj
        return obj

    def __parse_matching(self, match):
        objlist = []
        for filename in glob.iglob(match, recursive=True):
            yobj = self.__parse_file(filename)
            tobj = self.__process_yml_obj(yobj)
            objlist.append(tobj)
        return objlist

    def Parse(self, path, file_or_extn = '*'):
        match = utils.GetFullIglobPath(file_or_extn, path)
        logger.verbose("Processing file based on Match %s" % match)
        return self.__parse_matching(match)


def ParseFile(path, filename):
    parser = ParserBase()
    objlist = parser.Parse(path, filename)
    if len(objlist):
        return objlist[0]
    return None

def ParseDirectory(path, extn):
    parser = ParserBase()
    return parser.Parse(path, extn)
