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
    MUSTACHE_ERR_ALLOC,
    MUSTACHE_ERR_FILE_OPEN,
    MUSTACHE_ERR_NONEXISTENT,
    MUSTACHE_ERR_NO_SPACE,
    MUSTACHE_ERR_OVERFLOW,
    MUSTACHE_ERR_UNDERFLOW,
    MUSTACHE_ERR_INCOMPLETE,
    MUSTACHE_ERR_STREAM,
    MUSTACHE_ERR_ARGS,
    MUSTACHE_ERR_INVALID_CACHE,
    MUSTACHE_ERR_INVALID_TEMPLATE
} MUSTACHE_RES;

typedef enum {
    MUSTACHE_SEEK_SET = 0,
    MUSTACHE_SEEK_CUR = 1,
    MUSTACHE_SEEK_END = 2
} MUSTACHE_SEEK_DIR;

typedef enum {
    MUSTACHE_TYPE_VARIABLE,
    MUSTACHE_TYPE_FALSY,
    MUSTACHE_TYPE_POUND,
    MUSTACHE_TYPE_CLOSE,
    MUSTACHE_TYPE_COMMENT
} MUSTACHE_TYPE;

typedef enum {
    MUSTACHE_PARAM_NONE,
    MUSTACHE_PARAM_BOOLEAN,
    MUSTACHE_PARAM_NUMBER,
    MUSTACHE_PARAM_STRING,
    MUSTACHE_PARAM_LIST,
    MUSTACHE_PARAM_OBJECT
} MUSTACHE_PARAM_TYPE;

/* ===== STRUCTURE FORWARD DECLARATIONS */

typedef struct mustache_slice mustache_slice;

typedef struct mustache_const_slice mustache_const_slice;

typedef struct mustache_parser mustache_parser;

typedef struct mustache_param mustache_param;

typedef struct mustache_stream mustache_stream;

typedef struct mustache_structure mustache_structure;

/* ====== FUNCTION CALLBACK TYPES ====== */

typedef int32_t (*mustache_seek_callback)(void* udata, int64_t whence, MUSTACHE_SEEK_DIR seekdir);

typedef size_t (*mustache_read_callback)(void* udata, uint8_t* dst, size_t dstlen);

typedef void* (*mustache_alloc)(mustache_parser* parser, size_t bytes);

typedef void (*mustache_free)(mustache_parser* parser, void* block);


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
    void* userData;
    mustache_slice parentStackBuf;
    mustache_alloc alloc;
    mustache_free  free;
} mustache_parser;



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
    void* pValues;
    uint32_t valueCount;
} mustache_param_list;

typedef struct {
    void* pNext;
    MUSTACHE_PARAM_TYPE type;
    mustache_const_slice name;
    bool value;
} mustache_param_boolean;

typedef struct {
    void* pNext;
    MUSTACHE_PARAM_TYPE type;
    mustache_const_slice name;
    void* pMembers;
} mustache_param_object;

typedef struct {
    uint32_t begOffset;
    uint32_t lastOffset;
    uint32_t truthyLen;
} mustache_var_info;

typedef struct mustache_template_cache {
    mustache_slice varBuffer;

    uint32_t varCount;

    uint32_t privMAX_VAR_COUNT;
} mustache_template_cache;


typedef struct mustache_stream
{
    void* udata;
    mustache_read_callback readCallback;
    mustache_seek_callback seekCallback;
} mustache_stream;

typedef struct mustache_structure 
{
    void*           __A;
    void*           __B;
    MUSTACHE_RES    __C;
    uint32_t        __D;
    uint32_t        __C;
    uint32_t        __E;
    void*           __F;
    void*           __G;
} mustache_structure;
/* ====== FUNCTION CALLBACK TYPES ====== */

typedef void (*mustache_parse_callback)(mustache_parser* parser, void* udata, mustache_slice parsed);


/* ====== FUNCTIONS ====== */

uint8_t mustache_parse_file(mustache_parser* parser, mustache_const_slice filename, mustache_structure* structChain, mustache_param* params, mustache_slice sourceBuffer, mustache_slice parseBuffer, void* uData, mustache_parse_callback parseCallback);

uint8_t mustache_parse_stream(mustache_parser* parser, mustache_stream* stream, mustache_structure* structChain, mustache_param* params, mustache_slice sourceBuffer, mustache_slice parseBuffer, void* uData, mustache_parse_callback parseCallback);

void mustache_tructure_chain_free(mustache_structure* structure_chain);

#endif