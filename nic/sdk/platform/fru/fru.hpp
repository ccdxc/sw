// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __FRU_HPP__
#define __FRU_HPP__

#include <iostream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
namespace sdk {
namespace platform {

#define MAX_FRU_RETRIES 5

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#define FRU_FILE "/tmp/fru.json"
#define FRU_SIZE 256
#define MAC_LENGTH 18
#define UPPER_BYTE_MULTIPLIER 256
#define BYTE_MULTIPLIER 8

// top 2 bits represent field encoding
#define FIELD_ENCODING(x) (x >> 6 & 0x3)

#define ENGLISH_LANG_CODE0 0
#define ENGLISH_LANG_CODE1 25
#define LENGTH_FROM_FIELD 0x3F
#define ASCII_FIELD_ENCODING 0x3
#define END_OF_HEADER_AREA 0xC1
#define INVALID_LENGTH 0xC1

#define FORMAT_VER 0x01
#define PADDING_VALUE 0x00

// common info area
#define COM_HDR_VER_OFFSET 0
#define COM_HDR_BOARD_INFO_OFFSET 3
#define COM_HDR_PRODUCT_INFO_OFFSET 4
#define COM_HDR_PADDING_OFFSET 6
#define COM_HDR_LENGTH 8

// board info area
#define BOARD_INFO_AREA_MIN_LENGTH 13
#define BOARD_INFO_AREA_FORMAT_OFFSET 0
#define BOARD_INFO_AREA_LENGTH_OFFSET 1
#define BOARD_INFO_AREA_LANGUAGE_OFFSET 2
#define BOARD_INFO_AREA_DATE_OFFSET 3
#define BOARD_INFO_AREA_MANUFACTURING_OFFSET 6

#define BOARD_FRU_FILE_ID "frufileid"
#define BOARD_PARTNUM_KEY "part-number"
#define BOARD_PRODUCTNAME_KEY "product-name"
#define BOARD_MANUFACTURER_KEY "manufacturer"
#define BOARD_SERIALNUMBER_KEY "serial-number"
#define BOARD_MANUFACTURERDATE_KEY "manufacturing-date"

// custom board info area
#define BOARD_ID_KEY "board-id"
#define BOARD_MACADDRESS_KEY "mac-address"
#define BOARD_NUMMACADDR_KEY "num-mac-address"
#define BOARD_ENGCHANGELEVEL_KEY "engineering-change-level"
#define BOARD_ASSEMBLY_AREA_KEY "board-assembly-area"

// product info area
#define PRODUCT_INFO_AREA_MIN_LENGTH 12
#define PRODUCT_INFO_AREA_FORMAT_OFFSET 0
#define PRODUCT_INFO_AREA_LENGTH_OFFSET 1
#define PRODUCT_INFO_AREA_LANGUAGE_OFFSET 2
#define PRODUCT_INFO_AREA_MANUFACTURING_OFFSET 3

#define PRODUCT_PRODUCTNAME_KEY "product-name"
#define PRODUCT_PRODUCTNUMBER_KEY "product-part-number"
#define PRODUCT_VERSION_KEY "product-version"
#define PRODUCT_SERIALNUMBER_KEY "product-serial-number"
#define PRODUCT_MANUFACTURER_KEY "product-manufacturer"
#define PRODUCT_ASSETTAG_KEY "product-asset-tag"
#define PRODUCT_FRU_FILE_ID "product-frufileid"

int readfrukey(std::string key, std::string &value);

}    // namespace platform
}    // namespace sdk
#endif /* __FRU_HPP__ */
