#ifndef MUSTACHE_H
#define MUSTACHE_H

#include <stdint.h>
#include <stdbool.h>

/* ====== ENUM TYPES ====== */

typedef enum {
    MUSTACHE_SUCCESS=0,
    MUSTACHE_ERR,
    MUSTACHE_ERR_FILE,
    MUSTACHE_ERR_NONEXISTENT,
    MUSTACHE_ERR_NO_SPACE,
    MUSTACHE_ERR_OVERFLOW,
    MUSTACHE_ERR_UNDERFLOW
} mustache_res;

/* ===== STRUCTURE FORWARD DECLARATIONS */

typedef struct mustache_slice mustache_slice;

typedef struct mustache_const_slice mustache_const_slice;

typedef struct mustache_parser mustache_parser;

typedef struct mustache_cache_lookup_block mustache_cache_lookup_block;

typedef struct mustache_cache_item mustache_cache_item;

typedef struct mustache_cache_entry mustache_cache_entry;

typedef struct mustache_template_cache mustache_template_cache;

typedef struct mustache_param_list mustache_param_list;

typedef struct mustache_param_list mustache_param_list;


/* ====== FUNCTION CALLBACK TYPES ====== */

typedef void (*mustache_cache_entry_expiry_callback)(mustache_template_cache* cache, mustache_cache_entry* entry);


/* ====== STRUCTURE TYPES ====== */

typedef struct mustache_slice
{
    uint8_t* u;
    uint64_t len;
} mustache_slice;

typedef struct mustache_const_slice
{
    const uint8_t* u;
    uint64_t len;
} mustache_const_slice;


typedef struct mustache_parser
{
    mustache_slice buffer;
} mustache_parser;

typedef struct mustache_cache_lookup_block {
    uint32_t offset; // in bytes
    uint32_t capacity; //in bytes
    uint32_t entryCount;
} mustache_cache_lookup_block;

typedef struct mustache_cache_item
{
    uint8_t* data;
    uint32_t dataLen;
} mustache_cache_item;

typedef struct mustache_cache_entry {
    mustache_const_slice key;
    mustache_cache_item item;
} mustache_cache_entry;

typedef struct mustache_template_cache
{
    mustache_slice entryBuffer; // of type mustache_cache_entry
    mustache_cache_lookup_block firstByteLookup[256]; // corresponds to a byte offset within the entryBuffer buffer.
                                                      // a value of UINT32_MAX represents an unset offset.
    mustache_cache_entry_expiry_callback entryExpiryCallback;
} mustache_template_cache;

typedef struct mustache_param_list
{
    int t;
} mustache_param_list;


/* ====== FUNCTIONS ====== */

void mustache_template_cache_init(mustache_template_cache* cache);

bool mustache_check_cache(mustache_template_cache* cache, mustache_const_slice itemkey);

uint8_t mustache_cache_set_item(mustache_template_cache* cache, mustache_const_slice itemkey, mustache_cache_item item);

uint8_t mustache_cache_defragment(mustache_template_cache* cache);

uint8_t mustache_parse_file(mustache_parser* parser, mustache_const_slice filename,
     mustache_template_cache* templateCache, mustache_param_list* params);


#ifdef MUSTACHE_SYSTEM_TESTS
void mustache_cache_print_first_byte_lookup(const mustache_template_cache* cache);

void mustache_cache_print_entries_of_lookup_block(const mustache_template_cache* cache, uint8_t byte);

#endif

#endif