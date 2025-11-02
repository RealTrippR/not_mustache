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

#include <mustache.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

void parse_callback(mustache_parser* parser, void* udata, mustache_slice parsed)
{
    FILE* fptr = udata;
    fwrite(parsed.u, 1, parsed.len, fptr);
    return;
}

int main()
{
    uint8_t ITEM_BUFFER[16384];
    uint8_t PARSER_INPUT_BUFFER[4096];
    uint8_t PARSER_OUTPUT_BUFFER[8192];
    mustache_parser parser;


    mustache_param_boolean param_bool1 = {
        .pNext = NULL,
        .type = MUSTACHE_PARAM_BOOLEAN,
        .name = {"bool1",strlen("bool1")},
        .value = true
    };
    mustache_param_boolean param_bool2 = {
       .pNext = &param_bool1,
       .type = MUSTACHE_PARAM_BOOLEAN,
       .name = {"bool2",strlen("bool2")},
       .value = true
    };
    mustache_param_boolean param_bool3 = {
       .pNext = &param_bool2,
       .type = MUSTACHE_PARAM_BOOLEAN,
       .name = {"bool3",strlen("bool3")},
       .value = false
    };
    mustache_param_boolean param_bool4 = {
        .pNext = &param_bool3,
        .type = MUSTACHE_PARAM_BOOLEAN,
        .name = {"bool4",strlen("bool4")},
        .value = true
    };


    const char* filename = "conditionals.html";
    mustache_const_slice filenameSlice = { filename, strlen(filename) };


    FILE* fptr = fopen("conditionals_parsed.html", "wb");
    if (!fptr) {
        fprintf(stderr, "FAILED TO OPEN FILE\n");
        return -1;
    }


    uint8_t templateCacheBuf[8192];

    mustache_template_cache templateCache = {
        .varBuffer = {templateCacheBuf, sizeof(templateCacheBuf)}
    };

    if (mustache_parse_file(&parser, filenameSlice,
        &templateCache, MUSTACHE_CACHE_MODE_WRITE,
        (mustache_param*)&param_bool4,
        (mustache_slice) {
        PARSER_INPUT_BUFFER, sizeof(PARSER_INPUT_BUFFER)
    },
        (mustache_slice) {
        PARSER_OUTPUT_BUFFER, sizeof(PARSER_OUTPUT_BUFFER)
    },
        fptr, parse_callback) != MUSTACHE_SUCCESS)
    {
        fprintf(stderr, "MUSTACHE: FAILED TO PARSE FILE\n");
        return -1;
    }

    return 0;
}