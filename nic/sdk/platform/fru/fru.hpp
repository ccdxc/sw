// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __FRU_HPP__
#define __FRU_HPP__

#include <iostream>
#include "include/sdk/base.hpp"
#include "platform/pal/include/pal.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#define MAX_FRU_RETRIES 5

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#define FRU_FILE "/tmp/fru.json"
#define FRU_SIZE 256

//Predefined Keys
#define MANUFACTURERDATE_KEY "Manufacturing date"
#define MANUFACTURER_KEY "Manufacturer"
#define PRODUCTNAME_KEY "Product Name"
#define SERIALNUMBER_KEY "Serial Number"
#define PARTNUM_KEY "Part Number"
#define BOARDID_KEY "Board Id Number"
#define ENGCHANGELEVEL_KEY "Engineering Change level"
#define NUMMACADDR_KEY "NumMac Address"
#define MACADDRESS_KEY "Mac Address"

typedef struct common_fru_area
{
    uint8_t formatversion;      //0
    uint8_t internalareaoffset; //1
    uint8_t chassisareaoffset;  //2
    uint8_t boardinfooffset;    //3
    uint8_t productareaoffset;  //4
    uint8_t multrecareadoffset; //5
    uint8_t padding;            //6
    uint8_t checksum;           //7
} common_fru_area_t;

typedef struct board_fru_area
{
    uint8_t formatversion;        //8
    uint8_t length;               //9
    uint8_t languagecode;         //10
    uint8_t manufacturingdate[3]; //11
    uint8_t manufacturertype;     //14
    uint8_t manufacturer[21];     //15
    uint8_t productnametype;      //36
    uint8_t productname[10];      //37
    uint8_t reserved[6];          //47
    uint8_t serialnumtype;        //53
    uint8_t serialnum[11];        //54
    uint8_t partnumtype;          //65
    uint8_t partnum[13];          //66
    uint8_t frufileidtype;        //79
    uint8_t boardiftype;          //80
    uint8_t boardid[4];           //81
    uint8_t engchangeleveltype;   //85
    uint8_t engchangelevel[2];    //86
    uint8_t nmacaddrtype;         //88
    uint8_t nmacaddr[2];          //89
    uint8_t basemacaddrtype;      //91
    uint8_t macaddr[6];           //92
    uint8_t eof;                  //98
    uint8_t padding[4];           //99
    uint8_t checksum;             //103
} board_fru_area_t;

typedef struct fru_data
{
    common_fru_area_t commonfru;
    board_fru_area_t boardfru;
} fru_data_t;

int readKey(std::string key, std::string &value);

#endif /* __FRU_HPP__ */
