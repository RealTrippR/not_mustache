/***************************************************

Robins Free of Charge & Open Source Public License 25

Copyright (C), 2025 - Tripp R. All rights reserved.

Permission for this software, the "software" being source code, binaries, and documentation,
shall hereby be granted, free of charge, to be used for any purpose, including commercial applications,
modification, merging, and redistrubution. The software is provided 'as-is' and comes without any
express or implied warranty. This license is valid under the following restrictions:

1. The origin of the software must not be misrepresentented; only the true author(s) of the software
must be attributed as the it's creators. This applies every alteration of the "software", the name(s)
of the developer(s) of any alterations must be appended to the list of names of
the author(s) of the preceding version of the software which the alteration is based upon.

2. This license must be included in all redistributions of the software source.

3. All distrubitions of altered forms of the software must be clearly marked as such.

4. The author(s) of this software and all subsequent alterations hold no responsibility for any
damages that may result from use of the software.

5. The software shall not be used for the purpose of training LLMs ("Large Language Models"),
be included in datasets used for the purpose traning AI, or be used in the advancement of any
form of Artificial Intelligence.

***************************************************/

#define MUSTACHE_SYSTEM_TESTS

#include <not_mustache.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <crtdbg.h>

typedef struct
{
    void* block;
    size_t size;
    size_t capacity;
} parser_udata;

void* _alloc(mustache_parser* parser, size_t bytes) {
    parser_udata* udata = parser->userData;
    if (udata->size + bytes > udata->capacity) {
        return NULL;
    }
    udata->size += bytes;
    return (uint8_t*)udata->block + udata->size - bytes;
}


void _free(mustache_parser* parser, void* b) {
    //free(b);
}


void parse_callback(mustache_parser* parser, void* udata, mustache_slice parsed)
{
    FILE* fptr = udata;
    fwrite(parsed.u, 1, parsed.len, fptr);
    return;
}

int main()
{
#ifndef NDEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif /* !NDEBUG */

    printf("TODO: ADD XML PARSING SUPPORT.\n");

    uint8_t PARSER_INPUT_BUFFER[4096];
    uint8_t PARSER_OUTPUT_BUFFER[8192];
    uint8_t PARENT_STACK_BUFFER[2048];


    uint8_t PARSER_STRUCTURE_BUFFER[16384];
    parser_udata udata = { PARSER_STRUCTURE_BUFFER,0,sizeof(PARSER_STRUCTURE_BUFFER) };


    mustache_parser parser;
    parser.parentStackBuf = (mustache_slice){ PARENT_STACK_BUFFER,sizeof(PARENT_STACK_BUFFER) };
    parser.alloc = _alloc;
    parser.free = _free;
    parser.userData = &udata;

    mustache_param* jsonRoot;
    uint8_t err = mustache_JSON_to_param_chain_from_disk(&parser, (mustache_const_slice){ "test.json",strlen("test.json") }, &jsonRoot);
    if (err) {
        printf("FAILED TO PARSE JSON FILE");
        return -1;
    }


    mustache_print_parameter_list(jsonRoot);

    mustache_free_param_list(&parser, jsonRoot, true);

    return 0;
}