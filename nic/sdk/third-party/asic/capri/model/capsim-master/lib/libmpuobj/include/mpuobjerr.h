
/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __MPUOBJERR_H__
#define __MPUOBJERR_H__

#define MPUOBJERROR_GENERATOR(mac) \
    mac(MPUOBJERR_NONE,                     "no error") \
    mac(MPUOBJERR_SYSERR,                   "system error") \
    mac(MPUOBJERR_SHORT_FILE,               "short file") \
    mac(MPUOBJERR_INVALID_MAGIC,            "invalid file magic") \
    mac(MPUOBJERR_INVALID_FILE_HEADER,      "invalid file file header") \
    mac(MPUOBJERR_INVALID_FILE,             "invalid file") \
    mac(MPUOBJERR_INVALID_SECTION_HEADER,   "invalid section header") \
    mac(MPUOBJERR_INVALID_STRING_TABLE,     "invalid string table") \
    mac(MPUOBJERR_ELF_NO_TEXT,              "no ELF .text segment") \
    mac(MPUOBJERR_ELF_INVALID_SYMTAB,       "invalid symbol table") \
    mac(MPUOBJERR_ELF_INVALID_RELTAB,       "invalid relocation table") \
    mac(MPUOBJERR_INVALID_TEXT_SIZE,        "invalid text size")

#define MPUOBJERROR_ENUM_GEN(a, b) a,
enum MpuObjError {
    MPUOBJERROR_GENERATOR(MPUOBJERROR_ENUM_GEN)
};

#endif
