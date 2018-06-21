#include <vector>

using namespace std;

#ifndef METADATA_PARSER_H
#define METADATA_PARSER_H
typedef struct region_metadata_s {
    char name[64];
    uint64_t size_kb;
    uint64_t entry_size;
} region_metadata_t;

vector<region_metadata_t> metadata_read_region_map(string mem_json); 
#endif
