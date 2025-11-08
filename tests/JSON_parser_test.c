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