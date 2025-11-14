/******************************************************

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
be included in datasets used for the purpose of training AI, or be used in the advancement of any
form of Artificial Intelligence.

***************************************************/

#define MUSTACHE_SYSTEM_TESTS

#include <not_mustache.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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
    uint8_t PARSER_INPUT_BUFFER[4096];
    uint8_t PARSER_OUTPUT_BUFFER[8192];
    uint8_t PARENT_STACK_BUFFER[2048];

    uint8_t PARSER_STRUCTURE_BUFFER[65536];
    parser_udata udata = { PARSER_STRUCTURE_BUFFER,0,sizeof(PARSER_STRUCTURE_BUFFER)};

    mustache_parser parser;
    parser.parentStackBuf = (mustache_slice){ PARENT_STACK_BUFFER,sizeof(PARENT_STACK_BUFFER) };
    parser.alloc = _alloc;
    parser.free = _free;
    parser.userData = &udata;

    mustache_param_string param_title = {
        .pNext = NULL,
        .type = MUSTACHE_PARAM_STRING,
        .name = {"title",strlen("title")},
        .str = {"Generic Webpage",strlen("Generic Webpage")}
    };

    uint8_t* myName = "'<script> alert(\"you're hacked\") </script>'the name is Tripp";
    mustache_param_string param_name = {
       .pNext = &param_title,
       .type = MUSTACHE_PARAM_STRING,
       .name = {"name",strlen("name")},
       .str = {myName,strlen(myName)}
    };

    mustache_param_number param_number = {
      .pNext = &param_name,
      .type = MUSTACHE_PARAM_NUMBER,
      .name = {"messages",strlen("messages")},
      .value = 27,
      .decimals = 8,
      .trimZeros = true
    };

    mustache_param_boolean param_logged_in = {
        .pNext = &param_number,
        .type = MUSTACHE_PARAM_BOOLEAN,
        .name = {"loggedIn",strlen("loggedIn")},
        .value = true
    };

    mustache_param_boolean param_site_up = {
        .pNext = &param_logged_in,
        .type = MUSTACHE_PARAM_BOOLEAN,
        .name = {"site_up",strlen("site_up")},
        .value = true
    };

    mustache_param_string param_site = {
       .pNext = &param_site_up,
       .type = MUSTACHE_PARAM_STRING,
       .name = {"site",strlen("site")},
       .str = {"The World Wide Web",strlen("The World Wide Web")}
    };




    mustache_param_string name1 = {
        .pNext = NULL,
        .type = MUSTACHE_PARAM_STRING,
        .name = {"name",strlen("name")},
        .str = {"HowardAtNASA",strlen("HowardAtNASA")}
    };
    
    mustache_param_object userData1 = {
        .pNext = NULL,
         .type = MUSTACHE_PARAM_OBJECT,
        .name = {"data",strlen("data")},
        .pMembers = &name1
    };
    mustache_param_object user1 = {
        .pNext = NULL,
        .type = MUSTACHE_PARAM_OBJECT,
        .name = {"u1",strlen("u1")},
        .pMembers = &userData1
    };
    
    mustache_param_string name2 = {
       .pNext = NULL,
       .type = MUSTACHE_PARAM_STRING,
       .name = {"name",strlen("name")},
       .str = {"Elise_06",strlen("Elise_06")}
    };

    mustache_param_object userData2 = {
        .pNext = NULL,
         .type = MUSTACHE_PARAM_OBJECT,
        .name = {"data",strlen("data")},
        .pMembers = &name2
    };
    mustache_param_object user2 = {
        .pNext = &user1,
        .type = MUSTACHE_PARAM_OBJECT,
        .name = {"u2",strlen("u2")},
        .pMembers = &userData2
    };

    mustache_param_list param_list = {
        .pNext = &param_site,
        .type = MUSTACHE_PARAM_LIST,
        .name = {"users",strlen("users")},
        .valueCount = 2,
        .pValues = &user2
    };

    const char* filename = "basic.html";
    mustache_const_slice filenameSlice = { (const uint8_t*)filename, strlen(filename) };


    FILE* fptr = fopen("basic_parsed.html", "wb");
    if (!fptr) {
        fprintf(stderr,"FAILED TO OPEN FILE\n"); return -1;}




    mustache_structure struct_chain = {0};

    if (mustache_parse_file(&parser, filenameSlice, &struct_chain,
        (mustache_param*)&param_list,
        (mustache_slice){ PARSER_INPUT_BUFFER,sizeof(PARSER_INPUT_BUFFER) },
        (mustache_slice){ PARSER_OUTPUT_BUFFER,sizeof(PARSER_OUTPUT_BUFFER) },
        fptr, parse_callback) != MUSTACHE_SUCCESS)
    {
        // mustache_structure_chain_free()
        fprintf(stderr, "MUSTACHE: FAILED TO PARSE FILE\n");
        return -1;
    }



    // mustache_structure_chain_flush(&struct_chain); // <- this must be called before any
    // calls to mustache_parse_file or mustache_parse_stream if the parameters for this
    // structure chain have been invalidated or changed addresses.

    // mustache_structure_chain_free() <- if malloc was used for every node in the structure
    // chain rather than a unified stack buffer is in the example here, this function would
    // need to be called to release allocated memory.

    fclose(fptr);

    return 0;
}