// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "include/sdk/base.hpp"
#include "platform/fru/fru.hpp"
#include "platform/pal/include/pal.h"

using namespace std;

namespace sdk {
namespace platform {

static boost::property_tree::ptree input;
static boost::property_tree::ptree output;

static string BOARD_INFO_AREA[] = {
    BOARD_MANUFACTURER_KEY,
    BOARD_PRODUCTNAME_KEY,
    BOARD_SERIALNUMBER_KEY,
    BOARD_PARTNUM_KEY,
    BOARD_FRU_FILE_ID
};

static string PENSANDO_CUSTOM_BOARD_INFO_AREA[] = {
    BOARD_ID_KEY,
    BOARD_ENGCHANGELEVEL_KEY,
    BOARD_NUMMACADDR_KEY,
    BOARD_MACADDRESS_KEY
};

static string PENSANDO_VER2_CUSTOM_BOARD_INFO_AREA[] = {
    BOARD_ID_KEY,
    BOARD_ENGCHANGELEVEL_KEY,
    BOARD_NUMMACADDR_KEY,
    BOARD_MACADDRESS_KEY,
    BOARD_ASSEMBLY_AREA_KEY
};

#if 0
    // TODO: property tree problem
static string PRODUCT_INFO_AREA[] = {
    PRODUCT_MANUFACTURER_KEY,
    PRODUCT_PRODUCTNAME_KEY,
    PRODUCT_PRODUCTNUMBER_KEY,
    PRODUCT_VERSION_KEY,
    PRODUCT_SERIALNUMBER_KEY,
    PRODUCT_ASSETTAG_KEY,
    PRODUCT_FRU_FILE_ID
};
#endif

static bool validatechecksum(const uint8_t *arr, int length)
{
    uint32_t sum = 0;

    for (int i = 0; i < length; i++) {
        sum += arr[i];
    }

    if ((sum & 0xff) == 0) {
        return true;
    }
    return false;
}

static void gettime(const string key, uint8_t *board_date)
{
    struct tm timeinfo;
    time_t mantime = 0;

    //get time from 1/1/1996
    memset(&timeinfo, 0, sizeof(timeinfo));
    timeinfo.tm_year = 1996 - 1900;
    timeinfo.tm_mon =  0;
    timeinfo.tm_mday = 1;
    timeinfo.tm_hour = 0;
    timeinfo.tm_min =  0;
    timeinfo.tm_sec =  0;

    //since time in the fru is minutes.
    mantime  = 60 * (0x10000 * board_date[2] + 0x100 * board_date[1] + board_date[0]);
    mantime += mktime(&timeinfo);
    output.put(key, to_string(mantime));
}

static uint32_t store_field(const string key, uint8_t *fru_data,
                            uint32_t remaining_len)
{
    uint32_t len;

    len = fru_data[0] & LENGTH_FROM_FIELD;
    // Check if length is valid, 0xC1 represents end of field
    if (len == INVALID_LENGTH ||
        len > remaining_len) {
        return -1;
    }

    if (!len) {
        output.put(key, "");
        return 1;
    }

    // Check field encoding, only support ASCII encoding for now
    if (FIELD_ENCODING(fru_data[0]) == ASCII_FIELD_ENCODING) {
        string str((char *)&fru_data[1], len);
        str.erase(std::find(str.begin(), str.end(), '\0'), str.end());
        output.put(key, str);
    } else {
        output.put(key, "");
    }
    return len + 1;
}

static uint32_t store_customboard_field(const string key, uint8_t *fru_data,
                                        uint32_t remaining_len)
{
    uint32_t len;
    string str = "";

    len = fru_data[0] & LENGTH_FROM_FIELD;
    // Check if length is valid, 0xC1 represents end of field
    if (len == INVALID_LENGTH ||
        len > remaining_len) {
        return -1;
    }

    if (!len) {
        output.put(key, str);
        return 1;
    }

    if (key == BOARD_ID_KEY) {
        str = to_string(fru_data[4] << 24 | fru_data[3] << 16 |
                        fru_data[2] << 8 | fru_data[1]);
        output.put(key, str);
    } else if (key == BOARD_MACADDRESS_KEY) {
        snprintf((char*)str.c_str(), MAC_LENGTH, "%02x:%02x:%02x:%02x:%02x:%02x",
                fru_data[1], fru_data[2],fru_data[3],
                fru_data[4], fru_data[5], fru_data[6]);
        output.put(key, str.c_str());
    } else if (key == BOARD_NUMMACADDR_KEY) {
        str = to_string(fru_data[2] * UPPER_BYTE_MULTIPLIER + fru_data[1]);
        output.put(key, str);
    } else if (key == BOARD_ENGCHANGELEVEL_KEY) {
        str = to_string(fru_data[2] << 8 | fru_data[1]);
        output.put(key, str);
    } else if (key == BOARD_ASSEMBLY_AREA_KEY) {
        string asm_str((char *)&fru_data[1], len);
        output.put(key, asm_str);
    } else {
        output.put(key, str);
    }
    return len + 1;
}

static int parseproductarea(uint8_t *product_fru_data, uint32_t startoffset, uint32_t size)
{
#if 0
    // TODO: property tree problem
    uint8_t *p;
    uint32_t i = 0;
    int vlen = 0;
    uint32_t counter;
#endif
    uint32_t len = 0;
    uint32_t prod_fru_length = 0;

    if (product_fru_data[PRODUCT_INFO_AREA_FORMAT_OFFSET] != FORMAT_VER) {
        SDK_TRACE_ERR("Product area format version is not correct");
        goto err;
    }

    prod_fru_length = (product_fru_data[PRODUCT_INFO_AREA_LENGTH_OFFSET] * BYTE_MULTIPLIER);
    if (prod_fru_length < PRODUCT_INFO_AREA_MIN_LENGTH ||
        startoffset + prod_fru_length > size) {
        SDK_TRACE_ERR("Product area length is not correct");
        goto err;
    }

    if (validatechecksum(product_fru_data, prod_fru_length) == false) {
        SDK_TRACE_ERR("Product area checksum failed");
        goto err;
    }

    if (product_fru_data[PRODUCT_INFO_AREA_LANGUAGE_OFFSET] != ENGLISH_LANG_CODE0 &&
        product_fru_data[PRODUCT_INFO_AREA_LANGUAGE_OFFSET] != ENGLISH_LANG_CODE1) {
        SDK_TRACE_ERR("Product area language is non-English");
        goto err;
    }

    // get last possible padding
    // last element is the checksum
    len = prod_fru_length - 2;
    while ((product_fru_data[len] == 0x00) && (len != 0))
        len--;
    if (len == 0 || product_fru_data[len] != END_OF_HEADER_AREA) {
        SDK_TRACE_ERR("Product info not terminated properly");
        goto err;
    }

#if 0
    // TODO: figure out why property tree doesnt write the json file
    // correctly
    // parse standard product info
    p = &product_fru_data[PRODUCT_INFO_AREA_MANUFACTURING_OFFSET];
    for (counter = 0;
         counter < ARRAY_SIZE(PRODUCT_INFO_AREA);
         counter++) {
        vlen = store_field(PRODUCT_INFO_AREA[counter], p + i,
                           prod_fru_length - (i + PRODUCT_INFO_AREA_MANUFACTURING_OFFSET));
        if (vlen == -1) {
            // incorrect FRU
            return -1;
        }
        i += vlen;
    }
#endif
    return 0;
err:
    return -1;
}

static int parseboardarea(uint8_t *board_fru_data, uint32_t startoffset, uint32_t size)
{
    uint8_t *p;
    uint32_t i = 0;
    uint32_t len = 0;
    uint32_t board_fru_length = 0;
    int vlen = 0;
    uint32_t counter;
    string part_num;

    if (board_fru_data[BOARD_INFO_AREA_FORMAT_OFFSET] != FORMAT_VER) {
        SDK_TRACE_ERR("Board area format version is not correct");
        goto err;
    }

    board_fru_length = (board_fru_data[BOARD_INFO_AREA_LENGTH_OFFSET] * BYTE_MULTIPLIER);
    if (board_fru_length < BOARD_INFO_AREA_MIN_LENGTH ||
        startoffset + board_fru_length > size) {
        SDK_TRACE_ERR("Board area length is not correct");
        goto err;
    }

    if (validatechecksum(board_fru_data, board_fru_length) == false) {
        SDK_TRACE_ERR("Board area check failed");
        goto err;
    }

    if (board_fru_data[BOARD_INFO_AREA_LANGUAGE_OFFSET] != ENGLISH_LANG_CODE0 &&
        board_fru_data[BOARD_INFO_AREA_LANGUAGE_OFFSET] != ENGLISH_LANG_CODE1) {
        SDK_TRACE_ERR("Board area language is non-English");
        goto err;
    }

    // get last possible padding
    // last element is the checksum
    len = board_fru_length - 2;
    while ((board_fru_data[len] == 0x00) && (len != 0))
        len--;
    if (len == 0 || board_fru_data[len] != END_OF_HEADER_AREA) {
        SDK_TRACE_ERR("Board info not terminated properly");
        goto err;
    }

    gettime(BOARD_MANUFACTURERDATE_KEY,
            &board_fru_data[BOARD_INFO_AREA_DATE_OFFSET]);

    // parse standard board info
    p = &board_fru_data[BOARD_INFO_AREA_MANUFACTURING_OFFSET];
    for (counter = 0;
         counter < ARRAY_SIZE(BOARD_INFO_AREA);
         counter++) {
        vlen = store_field(BOARD_INFO_AREA[counter], p + i,
                           board_fru_length - (i + BOARD_INFO_AREA_MANUFACTURING_OFFSET));
        if (vlen == -1) {
            // incorrect FRU
            return -1;
        }
        i += vlen;
    }

    // Check partnumber
    try {
        part_num = output.get<string>(BOARD_PARTNUM_KEY);
    } catch (exception const &ex) {
        SDK_TRACE_ERR("%s",ex.what());
        return -1;
    }

    // New part number starts with "DSC"
    if (part_num.find("DSC") == 0) {
        // parse custom board info ver2
        for (counter = 0;
            counter < ARRAY_SIZE(PENSANDO_VER2_CUSTOM_BOARD_INFO_AREA);
            counter++) {
            vlen = store_customboard_field(PENSANDO_VER2_CUSTOM_BOARD_INFO_AREA[counter], p + i,
                           board_fru_length - (i + BOARD_INFO_AREA_MANUFACTURING_OFFSET));
            if (vlen == -1) {
                // incorrect FRU
                return -1;
            }
            i += vlen;
        }
    } else {
        // parse custom board info
        for (counter = 0;
            counter < ARRAY_SIZE(PENSANDO_CUSTOM_BOARD_INFO_AREA);
            counter++) {
            vlen = store_customboard_field(PENSANDO_VER2_CUSTOM_BOARD_INFO_AREA[counter], p + i,
                           board_fru_length - (i + BOARD_INFO_AREA_MANUFACTURING_OFFSET));
            if (vlen == -1) {
                // incorrect FRU
                return -1;
            }
            i += vlen;
        }
    }
    return 0;

err:
    return -1;
}

static int parsefru(uint8_t *fru_data, uint32_t size)
{
    if (size < FRU_SIZE) {
        SDK_TRACE_ERR("Size of the buffer is too small");
        goto err;
    }

    // Check FRU version
    if (fru_data[COM_HDR_VER_OFFSET] != FORMAT_VER) {
        SDK_TRACE_ERR("FRU Version mismatch");
        goto err;
    }

    // Check Padding
    if (fru_data[COM_HDR_PADDING_OFFSET] != PADDING_VALUE) {
        SDK_TRACE_ERR("FRU byte 6 should be PAD, and be zero");
        goto err;
    }

    // Check header checksum
    if (validatechecksum(fru_data, COM_HDR_LENGTH) == false) {
        SDK_TRACE_ERR("Common area check failed");
        goto err;
    }

    // Parse Board Area
    if (fru_data[COM_HDR_BOARD_INFO_OFFSET]) {
        if (fru_data[COM_HDR_BOARD_INFO_OFFSET] * BYTE_MULTIPLIER < size) {
            if (parseboardarea(&fru_data[fru_data[COM_HDR_BOARD_INFO_OFFSET] * BYTE_MULTIPLIER], 
                               fru_data[COM_HDR_BOARD_INFO_OFFSET] * BYTE_MULTIPLIER, size) != 0) {
                SDK_TRACE_ERR("Unable to parse board info area");
                goto err;
            }
        }
    }

    // Parse Product Area
    if (fru_data[COM_HDR_PRODUCT_INFO_OFFSET]) {
        if (fru_data[COM_HDR_PRODUCT_INFO_OFFSET] * BYTE_MULTIPLIER < size) {
            if (parseproductarea(&fru_data[fru_data[COM_HDR_PRODUCT_INFO_OFFSET] * BYTE_MULTIPLIER],
                                 fru_data[COM_HDR_PRODUCT_INFO_OFFSET] * BYTE_MULTIPLIER, size) != 0) {
                SDK_TRACE_ERR("Unable to parse product info area");
                goto err;
            }
        }
    }

    return 0;

err:
    return -1;
}

static int initFru()
{
    uint8_t *buffer;
    uint32_t nretry = MAX_FRU_RETRIES;

    buffer = (uint8_t *)calloc(1,FRU_SIZE);

    // read the FRU data
    if (pal_fru_read(buffer, FRU_SIZE, nretry) != 0) {
        return -1;
    }

    // parse fru
    if (parsefru(buffer, FRU_SIZE) != 0) {
        // unable to parse FRU exiting
        SDK_TRACE_ERR("Unable to parse FRU, exiting.");
        free(buffer);
        return -1;
    }
    boost::property_tree::write_json(FRU_FILE, output);

    free(buffer);
    return 0;
}

static int readfile(string Filename)
{
    try {
        boost::property_tree::read_json(Filename, input);
    }
    catch (exception const &ex) {
        SDK_TRACE_ERR("%s",ex.what());
        return -1;
    }
    return 0;
}

static int openfile()
{
    if (readfile(FRU_FILE)) {
        if (!initFru()) {
            if (readfile(FRU_FILE)) {
                return -1;
            }
        }
    }
    return 0;
}

int readfrukey(string key, string &value)
{
    if (input.empty()) {
        if (openfile()) {
            return -1;
        }
    }

    try {
        value = input.get<string>(key);
    } catch (exception const &ex) {
        SDK_TRACE_ERR("%s",ex.what());
        return -1;
    }

    return 0;
}

}    //namespace platform
}    //namespace sdk
