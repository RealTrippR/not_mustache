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
} MUSTACHE_RES;

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

typedef struct mustache_stream mustache_stream;


/* ====== FUNCTION CALLBACK TYPES ====== */

typedef int (*mustache_seek_callback)(void* udata, int64_t whence, int seekdir);

typedef size_t (*mustache_read_callback)(void* udata, uint8_t* dst, size_t dstlen);


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
    int placeholder;
    //mustache_slice buffer;
} mustache_parser;

typedef struct mustache_cache_lookup_block {
    uint32_t offset; // in bytes
    uint32_t capacity; //in bytes
    mustache_cache_lookup_block* next;
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
} mustache_template_cache;

typedef struct mustache_param_list
{
    int t;
} mustache_param_list;

typedef struct mustache_stream
{
    void* udata;
    mustache_read_callback readCallback;
    mustache_seek_callback seekCallback;
} mustache_stream;

/* ====== FUNCTION CALLBACK TYPES ====== */

typedef void (*mustache_parse_callback)(mustache_parser* parser, void* udata, mustache_slice parsed);

typedef void (*mustache_parse_callback)(mustache_parser* parser, void* udata, mustache_slice parsed);

/* ====== FUNCTIONS ====== */

void mustache_template_cache_init(mustache_template_cache* cache);

bool mustache_check_cache(mustache_template_cache* cache, mustache_const_slice itemkey);

uint8_t mustache_cache_set_item(mustache_template_cache* cache, mustache_const_slice itemkey, mustache_cache_item item);

uint8_t mustache_cache_remove_item(mustache_template_cache* cache, mustache_const_slice itemKey);

uint8_t mustache_parse_file(mustache_parser* parser, mustache_const_slice filename, mustache_param_list* params, mustache_slice parseBuffer, void* uData, mustache_parse_callback parseCallback);

uint8_t mustache_parse_stream(mustache_parser* parser, mustache_stream* stream, mustache_param_list* params, mustache_slice parseBuffer, void* uData, mustache_parse_callback parseCallback);


#ifdef MUSTACHE_SYSTEM_TESTS
void mustache_cache_print_first_byte_lookup(const mustache_template_cache* cache);

void mustache_cache_print_entries_of_lookup_block(const mustache_template_cache* cache, uint8_t byte);

void mustache_cache_validate(const mustache_template_cache* cache);

void mustache_cache_print_entries(const mustache_template_cache* cache);
#endif

#endif