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

typedef struct 
{
    void* block;
    size_t size;
} parser_udata;

void* _alloc(mustache_parser* parser, size_t bytes) {
    parser_udata* udata = parser->userData;
    void* tmp = realloc(udata->block, udata->size+bytes);
    if (!tmp) {
        return NULL;
    }
    udata->size += bytes;
    udata->block = tmp;
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

    parser_udata udata = { NULL };

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

    uint8_t* myName = "'<script> alert(\"&you're hacked\") </script>'the name is Tripp";
    mustache_param_string param_name = {
       .pNext = &param_title,
       .type = MUSTACHE_PARAM_STRING,
       .name = {"name",strlen("name")},
       //.str = {"'Here to inject malicious HTML: <script> alert(\"&you're hacked\") </script> 'Tripp",strlen("Tripp")}
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
        fprintf(stderr, "MUSTACHE: FAILED TO PARSE FILE\n");
        if (udata.block) {
            free(udata.block);
        }
        return -1;
    }



    //mustache_structure_chain_flush(&struct_chain); // <- this must be called if the
    // parameters have changed since the last usage of the structure chain.

    if (udata.block) {
        free(udata.block);
    }

    //   mustache_structure_chain_free() <- if malloc was used for every node in the structure
    //   chain rather than a unified buffer is in the example here, this function would have
    //   to be called to release allocated memory.

    fclose(fptr);

    return 0;
}