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
    MUSTACHE_ERR_STACK_OVERFLOW,
    MUSTACHE_ERR_OVERFLOW,
    MUSTACHE_ERR_UNDERFLOW,
    MUSTACHE_ERR_INCOMPLETE,
    MUSTACHE_ERR_STREAM,
    MUSTACHE_ERR_ARGS,
    MUSTACHE_ERR_INVALID_TEMPLATE,
    MUSTACHE_ERR_INVALID_JSON
} MUSTACHE_RES;

typedef enum {
    MUSTACHE_LOCALE_ASCII=1,
    MUSTACHE_LOCALE_UTF8=2,
    MUSTACHE_LOCALE_UTF16=3,
    MUSTACHE_LOCALE_EBCDIC_1=10, // IBM EBCDIC CODEPAGE 1
    MUSTACHE_LOCALE_EBCDIC_2=11  // IBM EBCDIC CODEPAGE 2
} MUSTACHE_LOCALE;


typedef enum {
    MUSTACHE_SEEK_SET = 0,
    MUSTACHE_SEEK_CUR = 1,
    MUSTACHE_SEEK_END = 2,
    MUSTACHE_SEEK_LEN = 3
} MUSTACHE_SEEK_DIR;



typedef enum {
    MUSTACHE_PARAM_NONE=0,
    MUSTACHE_PARAM_BOOLEAN=1,
    MUSTACHE_PARAM_NUMBER=2,
    MUSTACHE_PARAM_STRING=4,
    MUSTACHE_PARAM_LIST=8,
    MUSTACHE_PARAM_OBJECT=16,
    MUSTACHE_PARAM_TEMPLATE=32,
    MUSTACHE_PARAM_ALL_BITS = 0x1F
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

typedef void (*mustache_err_callback)(mustache_parser* parser, char err_msg[256], const char* src, const char* src_end);

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
    mustache_err_callback err_callback;
    mustache_err_callback warn_callback;

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
    void* pValues; // a pointer to a linked list of mustache parameter objects 
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
    void* pMembers; // a pointer to a linked list of mustache parameter objects 
} mustache_param_object;

typedef struct {
    void* pNext;
    MUSTACHE_PARAM_TYPE type;
    mustache_const_slice name;

    void* parameters; // a pointer to a linked list of mustache parameter objects
    mustache_structure* structure;
    mustache_const_slice source;
    
    mustache_slice parentStackBuffer;
} mustache_param_template;


/*
The mustache_stream is interface structure for parsers to read from an input source.
Note that streams are deterministic - if the behavior of a stream changes, 
the mustache_structure chain built with it must be destroyed.
*/
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

typedef struct {
    bool copy_strings:1;
    bool use_parser_alloc_free:1;
    struct {
        bool trim_zeros: 1;
        uint8_t max_decimals:5;
    } numinfo;

    mustache_parser* parser;
    char*cur;

    void* buffer;
    size_t buffer_size;

    mustache_param* first_param;

    
} mustache_json_info;

typedef struct {
    size_t max_line_size;
    size_t max_elements_per_object;
    size_t max_elements_per_array;
    size_t max_string_len;
    size_t max_num_len;

    MUSTACHE_LOCALE* supported_locales;
    uint16_t         supported_locale_count;
} mustache_parsing_engine_info;


/* ====== FUNCTION CALLBACK TYPES ====== */

typedef void (*mustache_parse_callback)(mustache_parser* parser, void* udata, mustache_slice parsed);


/* ====== FUNCTIONS ====== */


mustache_param* mustache_param_get_child_at_index(mustache_param*, uint32_t index);

/*****
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-

-+- Parses a mustache template source from disk. -+-

@param mustache_parser* parser
@param mustache_const_slice filename
@param mustache_structure* structChain - a pointer to a chain of mustache structures. Structure chains be reused, but only if the addresses, source template, and types of variables remain constant. Call mustache_structure_chain_flush to reset a structure chain that has been invalidated.
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
@param mustache_structure* structChain - a pointer to a chain of mustache structures. Structure chains be reused, but only if the addresses, source template, and types of variables remain constant. Call mustache_structure_chain_flush to reset a structure chain that has been invalidated.
@param mustache_param* params - the parameter chain
@param mustache_slice sourceBuffer - if the stream length is larger than the source buffer, mustache_parse_file will return ERR_NO_SPACE
@param mustache_slice parseBuffer - where the parsed template will be stored
@param void* parseCallbackUdata - passed to the parseCallback function
@param mustache_parse_callback - called upon parse completion.

@return uint8_t - MUSTACHE_RES return code.      
      
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*****/
uint8_t mustache_parse_stream(mustache_parser* parser, mustache_slice parentStackBuffer, mustache_stream* stream, mustache_structure* structChain, mustache_param* params, mustache_slice inputBuffer, mustache_slice parseOutputBuffer, void* parseCallbackUdata, mustache_parse_callback parseCallback);


/*****
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-

-+- Destroys a structure chain, calling parser->free for every node in the list. -+-

This must be called when:
- a structure chain is no longer needed
- the contents of the template source used to build it have changed
- the stream used in building it has changed

@param mustache_parser* parser
@param mustache_structure* structure_chain

-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*****/
void mustache_structure_chain_free(mustache_parser* parser, mustache_structure* structure_chain);


/*****
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-

-+- Primes a structure chain for its next use, this must be called if the parameter -+-
    chain used to generate this structure chain has nodes that were invalidated, changed
    type, or changed addresses since the last call to mustache_parse_file or 
    mustache_parse_stream. The source template that was used to generate the structure
    chain must also remain constant.

@param mustache_structure* structure_chain

-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*****/
void mustache_structure_chain_flush(mustache_structure* structure_chain);



/*****
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*****/
void mustache_structure_chain_update(mustache_structure* chain, void** updated_parameters, uint32_t updated_param_count);


/*****
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*****/
mustache_param* mustache_parameter_get_child_list(mustache_param* param, uint32_t* max_child_count);


/*****
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*****/
char mustache_get_stream_range(int32_t input_buffer_len, const char** first, const char** end, const char* src_first, const char* src_end);


/*****
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*****/
void mustache_dummy_err_callback(mustache_parser* parser, char err_msg[256],const char* src,const char* src_end);





/*****
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-

-+- Converts JSON into a mustache parameter chain. -+-

@param mustache_parser* parser
@param mustache_const_slice JSON source
@param mustache_json_info* json_info
@return MUSTACHE_RES


    - if json_info.use_parser_alloc_free is false,
    - the required size needed for a buffer will be
    - set on call.
    - the buffer allocated for the parameters must be of 
    - size json_info.buffer_size, no bounds checks
    - on the param buffer.


-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*****/
uint8_t mustache_JSON(mustache_parser* parser, mustache_const_slice JSON, mustache_json_info* json_info);



/*****
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-

-+- Frees a parameter chain created with mustache_JSON. -+- 

If json_info.use_parser_alloc_free is false,
this function need not be called, but it is safe to do so regardless.

acts as a no-op is json_info.buffer is NULL or if json_info.use_parser_alloc_free is false

@param mustache_parser* parser
@param mustache_json_info* json_info

@return MUSTACHE_RES

-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*****/
uint8_t mustache_JSON_free(mustache_parser* parser, mustache_json_info* json_info);

/*****
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
Queries JSON parsing engine parameters.
*****/
const mustache_parsing_engine_info* mustache_JSON_get_parsing_engine_info();

/*
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-

                                 SYSTEM TESTS & DEBUG TOOLS

-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*/

#define MUSTACHE_SYSTEM_TESTS
#ifdef MUSTACHE_SYSTEM_TESTS

void mustache_print_node(mustache_param* node, int depth);

void mustache_print_parameter_list(mustache_param* root);

#endif

#ifndef NDEBUG 

void mustache_dbg_print_structure_chain(mustache_structure* structure, int32_t tab_depth);

mustache_param** mustache_dbg_structure_get_params(mustache_structure* structure, uint32_t *pcount, void**);

#endif

#endif