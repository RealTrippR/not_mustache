/*
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- 

Robins Free of Charge & Open Source Public License 25

Copyright (C), 2025 - Tripp R. All rights reserved.

Permission for this software, the "software" being source code, binaries, and documentation,
shall hereby be granted, free of charge, to be used for any purpose, including commercial applications,
modification, merging, and redistrubution. The software is provided 'as-is' and comes without any
express or implied warranty. This license is valid under the following restrictions:

1. The origin of the software must not be misrepresentented; the true author(s) of the software
must be attributed as such. This applies every alteration of the "software", the name(s)
of the authors(s) of any alterations must be appended to the list of names of
the author(s) of the preceding version of the software which the alteration is based upon.

2. This license must be included in all redistributions of the software source.

3. All distributions of altered forms of the software must be clearly marked as such.

4. The author(s) of this software and all subsequent alterations hold no responsibility for any
damages that may result from use of the software.

5. The software shall not be used for the purpose of training LLMs ("Large Language Models"),
be included in datasets used for the purpose of training AI, or be used in the advancement of any
form of Artificial Intelligence.

-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*/


/*
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-

PREPROCESSOR FLAGS:
- NOT_MUSTACHE_TARGET_MSVC <- define if targeting the MSVC or Odin compiler.

-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*/


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
    MUSTACHE_ERR_INVALID_TEMPLATE,
    MUSTACHE_ERR_INVALID_JSON
} MUSTACHE_RES;

typedef enum {
    MUSTACHE_SEEK_SET = 0,
    MUSTACHE_SEEK_CUR = 1,
    MUSTACHE_SEEK_END = 2,
    MUSTACHE_SEEK_LEN = 3
} MUSTACHE_SEEK_DIR;



typedef enum {
    MUSTACHE_PARAM_NONE,
    MUSTACHE_PARAM_BOOLEAN,
    MUSTACHE_PARAM_NUMBER,
    MUSTACHE_PARAM_STRING,
    MUSTACHE_PARAM_LIST,
    MUSTACHE_PARAM_OBJECT,
    MUSTACHE_PARAM_TEMPLATE
} MUSTACHE_PARAM_TYPE;

/* ===== STRUCTURE FORWARD DECLARATIONS */

typedef struct mustache_slice mustache_slice;

typedef struct mustache_const_slice mustache_const_slice;

typedef struct mustache_parser mustache_parser;

typedef struct mustache_param mustache_param;

typedef struct mustache_stream mustache_stream;

typedef struct mustache_structure mustache_structure;

/* ====== FUNCTION CALLBACK TYPES ====== */

typedef uint64_t (*mustache_seek_callback)(void* udata, int64_t whence, MUSTACHE_SEEK_DIR seekdir);

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
    mustache_alloc alloc;
    mustache_free  free;
    
    uint8_t spacesPerTab;
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
    void* pNext;
    MUSTACHE_PARAM_TYPE type;
    mustache_const_slice name;

    void* parameters;
    mustache_structure* structure;
    mustache_const_slice source;
    
    mustache_slice parentStackBuffer;
} mustache_param_template;




typedef struct mustache_stream
{
    void* udata;
    mustache_read_callback readCallback;
    mustache_seek_callback seekCallback;
} mustache_stream;

typedef struct mustache_structure 
{
    void*           __A;        /* DO NOT ATTEMPT TO MODIFY THIS MEMBER, IT IS A PLACEHOLDER */
    void*           __B;        /* DO NOT ATTEMPT TO MODIFY THIS MEMBER, IT IS A PLACEHOLDER */
    MUSTACHE_RES    __C;        /* DO NOT ATTEMPT TO MODIFY THIS MEMBER, IT IS A PLACEHOLDER */
    uint32_t        __D;        /* DO NOT ATTEMPT TO MODIFY THIS MEMBER, IT IS A PLACEHOLDER */
    uint32_t        __E;        /* DO NOT ATTEMPT TO MODIFY THIS MEMBER, IT IS A PLACEHOLDER */
    uint32_t        __F;        /* DO NOT ATTEMPT TO MODIFY THIS MEMBER, IT IS A PLACEHOLDER */
    void*           __G;        /* DO NOT ATTEMPT TO MODIFY THIS MEMBER, IT IS A PLACEHOLDER */
    void*           __H;        /* DO NOT ATTEMPT TO MODIFY THIS MEMBER, IT IS A PLACEHOLDER */
} mustache_structure;

/* ====== FUNCTION CALLBACK TYPES ====== */

typedef void (*mustache_parse_callback)(mustache_parser* parser, void* udata, mustache_slice parsed);


/* ====== FUNCTIONS ====== */

/*****
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-

-+- Parses a mustache template source from disk. -+-

@param mustache_parser* parser
@param mustache_const_slice filename
@param mustache_structure* structChain - a pointer to a chain of mustache structures
@param mustache_param* params - the parameter chain
@param mustache_slice sourceBuffer - if the file length is larger than the source buffer, mustache_parse_file will return ERR_NO_SPACE
@param mustache_slice parseBuffer - where the parsed template will be stored
@param void* parseCallbackUdata - passed to the parseCallback function
@param mustache_parse_callback - called upon parse completion

@return uint8_t - MUSTACHE_RES return code.

-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*****/
uint8_t mustache_parse_file(mustache_parser* parser, mustache_slice parentStackBuffer,  mustache_const_slice filename, mustache_structure* structChain, mustache_param* params, mustache_slice sourceBuffer, mustache_slice parseBuffer, void* parseCallbackUdata, mustache_parse_callback parseCallback);

/*****
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-

-+- Parses a mustache template source from an input stream. -+-

@param mustache_parser* parser
@param mustache_slice parentStackBuffer - a stack to hold the parent context(s)
@param mustache_stream - the input stream to parse from
@param mustache_structure* structChain - a pointer to a chain of mustache structures
@param mustache_param* params - the parameter chain
@param mustache_slice sourceBuffer - if the stream length is larger than the source buffer, mustache_parse_file will return ERR_NO_SPACE
@param mustache_slice parseBuffer - where the parsed template will be stored
@param void* parseCallbackUdata - passed to the parseCallback function
@param mustache_parse_callback - called upon parse completion.

@return uint8_t - MUSTACHE_RES return code.      
      
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*****/
uint8_t mustache_parse_stream(mustache_parser* parser, mustache_slice parentStackBuffer, mustache_stream* stream, mustache_structure* structChain, mustache_param* params, mustache_slice sourceBuffer, mustache_slice parseBuffer, void* parseCallbackUdata, mustache_parse_callback parseCallback);


/*****
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-

-+- Destroys a structure chain, calling parser->free for every node in the list. -+-

@param mustache_parser* parser
@param mustache_structure* structure_chain

-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*****/
void mustache_structure_chain_free(mustache_parser* parser, mustache_structure* structure_chain);


/*****
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-

-+- Primes a structure chain for its next use, this must be called if the parameter -+-
    chain used to generate this structure chain has nodes that were invalidated
    or changed addresses since the last call to mustache_parse_file or 
    mustache_parse_stream.

@param mustache_structure* structure_chain

-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*****/
void mustache_structure_chain_flush(mustache_structure* structure_chain);

/*****
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-

-+- Converts JSON from a file on disk into a mustache parameter chain. -+-
@param mustache_parser* parser
@param mustache_const_slice filename
@param mustache_param** paramRoot - pointer to a pointer to the beginning of the parameter chain.

@return uint8_t - MUSTACHE_RES return code.

-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*****/
uint8_t mustache_JSON_to_param_chain_from_disk(mustache_parser* parser, mustache_const_slice filename, mustache_param** paramRoot);


/*****
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-

-+- Converts JSON into a mustache parameter chain. -+-

@param mustache_parser* parser
@param mustache_const_slice - JSON source
@param mustache_param** paramRoot - pointer to a pointer 
@param bool deepCopyData - if true, the source data will be copied rather than shallowly referenced where applicable.

@return uint8_t - MUSTACHE_RES return code.

-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*****/
uint8_t mustache_JSON_to_param_chain(mustache_parser* parser, mustache_const_slice JSON, mustache_param** paramRoot, bool deepCopyData);



/*****
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-

-+- Converts JSON into a mustache parameter chain. -+-

@param mustache_parser* parser
@param mustache_const_slice - JSON source
@param mustache_param* paramRoot - the parameter root
@param bool deepCopyData - determines what data will be freed.

@return uint8_t - MUSTACHE_RES return code.

-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*/
uint8_t mustache_free_param_list(mustache_parser* parser, mustache_param* paramRoot, bool deepCopy);

/*
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-

                                 SYSTEM TESTS & DEBUG TOOLS

-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*/

#ifdef MUSTACHE_SYSTEM_TESTS

void mustache_print_node(mustache_param* node, int depth);

void mustache_print_parameter_list(mustache_param* root);

#endif

#endif