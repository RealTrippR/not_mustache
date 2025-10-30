/***************************************************
MIT License

Copyright (C) 2025 Tripp R

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
***************************************************/

#ifndef MUSTACHE_H
#define MUSTACHE_H

#include <stdint.h>
#include <stdbool.h>

/* ====== ENUM TYPES ====== */

typedef enum {
    MUSTACHE_SUCCESS=0,
    MUSTACHE_ERR,
    MUSTACHE_ERR_FILE_OPEN,
    MUSTACHE_ERR_NONEXISTENT,
    MUSTACHE_ERR_NO_SPACE,
    MUSTACHE_ERR_OVERFLOW,
    MUSTACHE_ERR_UNDERFLOW,
    MUSTACHE_ERR_ARGS
} MUSTACHE_RES;

typedef enum {
    MUSTACHE_SEEK_SET = 0,
    MUSTACHE_SEEK_CUR = 1,
    MUSTACHE_SEEK_END = 2
} MUSTACHE_SEEK_DIR;

typedef enum {
    MUSTACHE_PARAM_NONE,
    MUSTACHE_PARAM_BOOLEAN,
    MUSTACHE_PARAM_NUMBER,
    MUSTACHE_PARAM_STRING
} MUSTACHE_PARAM_TYPE;

/* ===== STRUCTURE FORWARD DECLARATIONS */

typedef struct mustache_slice mustache_slice;

typedef struct mustache_const_slice mustache_const_slice;

typedef struct mustache_parser mustache_parser;

typedef struct mustache_cache_lookup_block mustache_cache_lookup_block;

typedef struct mustache_cache_item mustache_cache_item;

typedef struct mustache_cache_entry mustache_cache_entry;

typedef struct mustache_cache mustache_cache;

typedef struct mustache_template_cache  mustache_template_cache;

typedef struct mustache_param mustache_param;

typedef struct mustache_stream mustache_stream;

typedef struct mustache_template_cache_entry mustache_template_cache_entry;


/* ====== FUNCTION CALLBACK TYPES ====== */

typedef int32_t (*mustache_seek_callback)(void* udata, int64_t whence, MUSTACHE_SEEK_DIR seekdir);

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

typedef struct mustache_cache
{
    mustache_slice entryBuffer; // of type mustache_cache_entry
    mustache_cache_lookup_block firstByteLookup[256]; // corresponds to a byte offset within the entryBuffer buffer.
                                                      // a value of UINT32_MAX represents an unset offset.
} mustache_cache;


typedef struct mustache_param {
    void* pNext;
    MUSTACHE_PARAM_TYPE type;
    mustache_const_slice name;
} mustache_param;

typedef struct {
    void* pNext;
    MUSTACHE_PARAM_TYPE type;
    mustache_const_slice name;
    mustache_slice str;
} mustache_param_string;

typedef struct {
    void* pNext;
    MUSTACHE_PARAM_TYPE type;
    mustache_const_slice name;
    double value;
    uint8_t decimals;
    bool trimZeros;
} mustache_param_number;

typedef struct {
    void* pNext;
    MUSTACHE_PARAM_TYPE type;
    mustache_const_slice name;
    bool value;
} mustache_param_boolean;

typedef struct {
    uint32_t begOffset;
    uint32_t endOffset;
} mustache_var_info;

typedef struct mustache_template_cache {
    mustache_slice varBuffer;
    mustache_slice conditionalBuffer;

    uint32_t varCount;
    uint32_t conditionalCount;
} mustache_template_cache;

typedef struct mustache_stream
{
    void* udata;
    mustache_read_callback readCallback;
    mustache_seek_callback seekCallback;
} mustache_stream;

/* ====== FUNCTION CALLBACK TYPES ====== */

typedef void (*mustache_parse_callback)(mustache_parser* parser, void* udata, mustache_slice parsed);


/* ====== FUNCTIONS ====== */

void mustache_template_cache_init(mustache_cache* cache);

bool mustache_check_cache(mustache_cache* cache, mustache_const_slice itemkey);

uint8_t mustache_cache_set_item(mustache_cache* cache, mustache_const_slice itemkey, mustache_cache_item item);

uint8_t mustache_cache_remove_item(mustache_cache* cache, mustache_const_slice itemKey);

uint8_t mustache_parse_file(mustache_parser* parser, mustache_const_slice filename, mustache_param* params, mustache_slice sourceBuffer, mustache_slice parseBuffer, void* uData, mustache_parse_callback parseCallback);

uint8_t mustache_parse_stream(mustache_parser* parser, mustache_stream* stream, mustache_template_cache* streamCache, mustache_param* params, mustache_slice sourceBuffer, mustache_slice parseBuffer, void* uData, mustache_parse_callback parseCallback);


#ifdef MUSTACHE_SYSTEM_TESTS
void mustache_cache_print_first_byte_lookup(const mustache_cache* cache);

void mustache_cache_print_entries_of_lookup_block(const mustache_cache* cache, uint8_t byte);

void mustache_cache_validate(const mustache_cache* cache);

void mustache_cache_print_entries(const mustache_cache* cache);
#endif

#endif