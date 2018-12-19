// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "platform/fru/fru.hpp"

using namespace std;

static boost::property_tree::ptree input;
static bool validatefruchecksum(const uint8_t *arr, int length)
{
    uint32_t sum = 0;

    for (int i = 0; i < length; i++)
        sum += arr[i];

    if ((sum & 0xff) == 0) {
        return true;
    }
    return false;
}

static bool validatecommon(const common_fru_area_t *commonfru)
{
    return (validatefruchecksum((uint8_t*)commonfru, sizeof (*commonfru)));
}

static bool validateboard(const board_fru_area_t *boardfru)
{
    return (validatefruchecksum((uint8_t*)boardfru, sizeof (*boardfru)));
}

static bool validatefru(fru_data_t *fru)
{
    if (validatecommon(&fru->commonfru) &&
        validateboard(&fru->boardfru)) {
        return true;
    }
    return false;
}

static time_t gettime(uint8_t *date) {
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
    mantime  = 60 * (0x10000 * date[2] + 0x100 * date[1] + date[0]);
    mantime += mktime(&timeinfo);
    return mantime;
}
static uint8_t createfile(fru_data_t *fru)
{
    // Create a root
    boost::property_tree::ptree output;
    char manufacturingdate[64] = {0};
    char manufacturer[22] = {0};
    char productname[11] = {0};
    char serialnum[12] = {0};
    char partnum[14] = {0};
    char boardid[5] = {0};
    char engchangelevel[3] = {0};
    char nmacaddr[3];
    char macStr[18];

    snprintf(manufacturingdate, sizeof(manufacturingdate), "%ld"
             , gettime(fru->boardfru.manufacturingdate));
    output.put(MANUFACTURERDATE_KEY, manufacturingdate);

    snprintf(manufacturer, sizeof(manufacturer), "%s"
             , fru->boardfru.manufacturer);
    output.put(MANUFACTURER_KEY, manufacturer);

    snprintf(productname, sizeof(productname), "%s"
             , fru->boardfru.productname);
    output.put(PRODUCTNAME_KEY, productname);

    snprintf(serialnum, sizeof(serialnum), "%s"
             , fru->boardfru.serialnum);
    output.put(SERIALNUMBER_KEY, serialnum);

    snprintf(partnum, sizeof(partnum), "%s"
             , fru->boardfru.partnum);
    output.put(PARTNUM_KEY, partnum);

    snprintf(engchangelevel, sizeof(engchangelevel), "%d%d"
             , fru->boardfru.engchangelevel[0]
             , fru->boardfru.engchangelevel[1]);
    output.put(ENGCHANGELEVEL_KEY, engchangelevel);

    snprintf(boardid, sizeof(boardid), "%d%d%d%d"
             , fru->boardfru.boardid[0]
             , fru->boardfru.boardid[1]
             , fru->boardfru.boardid[2]
             , fru->boardfru.boardid[3]);
    output.put(BOARDID_KEY, boardid);

    snprintf(nmacaddr, sizeof(nmacaddr), "%d",
             (int)fru->boardfru.nmacaddr[1] * 256 + fru->boardfru.nmacaddr[0]);
    output.put(NUMMACADDR_KEY, nmacaddr);

    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
            fru->boardfru.macaddr[0],
            fru->boardfru.macaddr[1],
            fru->boardfru.macaddr[2],
            fru->boardfru.macaddr[3],
            fru->boardfru.macaddr[4],
            fru->boardfru.macaddr[5]);
    output.put(MACADDRESS_KEY, macStr);

    boost::property_tree::write_json(FRU_FILE, output);

    return 0;
}

static int initFru()
{
    uint8_t *buffer;
    fru_data_t *fru;
    uint32_t nretry = MAX_FRU_RETRIES;

    buffer = (uint8_t*)malloc(FRU_SIZE);

    pal_fru_read(buffer, FRU_SIZE, nretry);
    fru = (fru_data_t *)buffer;
    if (validatefru(fru) == FALSE) {
        // unable to validate FRU exiting
        cout << "Unable to validate FRU, exiting." << endl;
        return -1;
    }

    // create fru.json
    if (createfile(fru))
    {
        return -1;
    }

    return 0;
}

static int readfile(string Filename)
{
    try {
        boost::property_tree::read_json(Filename, input);
    }
    catch (std::exception const &ex) {
        cout << ex.what() << endl;
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

int readKey(std::string key, std::string &value)
{
    if (input.empty()) {
        if (openfile()) {
            return -1;
        }
    }
    if (input.get_optional<std::string>(key)) {
        value = input.get<std::string>(key);
    }
    else {
        return -1;
    }
    return 0;
}

int readMac(std::string &value)
{
    return readKey(MACADDRESS_KEY, value);
}

int readNumberMacs(std::string &value)
{
    return readKey(NUMMACADDR_KEY, value);
}
