#include <stdio.h>
#include <stdlib.h>

#include <not_mustache/not_mustache.h>
#include <not_mustache/not_mustache_pmacros.h>

typedef struct 
{
    void* block;
    size_t size; // used size
    size_t capacity; // total capacity
} parser_udata;
parser_udata parserudata;

typedef struct
{
    const unsigned char* data;
    size_t cur;
    size_t len;
} stream_udata;
stream_udata streamudata;


void* parser_alloc(mustache_parser* parser, size_t bytes) {
    parser_udata* udata = parser->userData;
    if (udata->size + bytes > udata->capacity) {
        return NULL;
    }
    udata->size += bytes;
    return (uint8_t*)udata->block + udata->size - bytes;
}


void parser_free(mustache_parser* parser, void* b) {
    //free(b);
}




mustache_structure struct_chain = {0};
mustache_parser parser = {
    .alloc = parser_alloc,
    .free = parser_free,
    .spacesPerTab = 10,
    .userData = &parserudata
};


size_t stream_read_callback(void* u, uint8_t* dst, size_t dstlen) {
    stream_udata* udata = u;
    size_t cpylen;
    if (udata->len-udata->cur < dstlen) {
        cpylen=udata->len-udata->cur;
    } else {
        cpylen = dstlen;
    }
    memcpy(dst, udata->data + udata->cur, cpylen);
    return cpylen;
}

uint64_t stream_seek_callback(void* u, int64_t whence, MUSTACHE_SEEK_DIR seekdir) {
    stream_udata* udata = u;
    switch (seekdir)
    {
    case MUSTACHE_SEEK_LEN:
        return udata->len;
    case MUSTACHE_SEEK_SET:
        udata->cur=whence;
        return udata->cur;
    case MUSTACHE_SEEK_END:
        if (whence < 0 && -whence >= udata->len) {
            udata->cur = 0;
            return 0;
        }
        udata->cur=udata->len+whence-1;
        if (udata->cur >= udata->cur+udata->len) {
            udata->cur = udata->cur+udata->len-1;
        }
        return udata->cur;
    default:
        return 0;
    }
}



mustache_stream parser_stream = {
    .readCallback = stream_read_callback,
    .seekCallback = stream_seek_callback,
    .udata = &streamudata,
};

void dummy_write_parsed_data_callback(mustache_parser* parser, void* udata, mustache_slice parsed) {
    (void)parser;
    *(size_t*)udata = parsed.len;
    (void)parsed;
}


/*
parses a template source and stores it in a dynamically allocated buffer.

@param const char* template_source: The not-mustache template source as a null-terminated string
@param char** parsed_template: ptr to a ptr to the parsed template. On success will be set to a dynamically allocated buffer containing the parsed template as a NON null terminated string.
@param size_t* parsed_template_length: On success, this will be set to the length of the parsed template
@param void* arglist: A linked list of the mustache parameters to be fed into the template.
*/
MUSTACHE_RES parse_template(const char* template_source, char** parsed_template, size_t* parsed_template_length, void* arglist)
{
    uint8_t PARSER_INPUT_BUFFER[4096];
    uint8_t PARSER_OUTPUT_BUFFER[8192];
    uint8_t PARENT_STACK_BUFFER[2048];

    uint8_t PARSER_STRUCTURE_BUFFER[65536];
    parserudata = (parser_udata){ .block = PARSER_STRUCTURE_BUFFER, .size = 0, .capacity = sizeof(PARSER_STRUCTURE_BUFFER)};
    streamudata = (stream_udata){ .data = template_source, .cur = 0, .len = strlen(template_source) };


    *parsed_template = NULL;

    MUSTACHE_RES m = mustache_parse_stream(
        &parser,
        (mustache_slice){PARENT_STACK_BUFFER, sizeof(PARENT_STACK_BUFFER)},
        &parser_stream,
        &struct_chain,
        arglist,
        (mustache_slice){PARSER_INPUT_BUFFER, sizeof(PARSER_INPUT_BUFFER)},
        (mustache_slice){PARSER_OUTPUT_BUFFER, sizeof(PARSER_OUTPUT_BUFFER)},
        parsed_template_length,
        dummy_write_parsed_data_callback);

    if (m<0) {
        return m;
    } else {
        if (*parsed_template_length == 0) {
            return MUSTACHE_SUCCESS;
        }
        *parsed_template = malloc(*parsed_template_length);
        if (!*parsed_template) {
            return MUSTACHE_ERR_ALLOC;
        }

        memcpy(*parsed_template,PARSER_OUTPUT_BUFFER,*parsed_template_length);

        return m;
    }
}


MUSTACHE_RES free_template(char* parsed_template)
{
    mustache_structure_chain_free(&parser, &struct_chain);
    free(parsed_template);
    return MUSTACHE_SUCCESS;
}


void MIDVLPARAM_TO_STR_C_ALLOC(void* vparam, char* write, size_t* size);
void MPARAM_LIST_TO_STR_C_ALLOC(mustache_param_list* p, char* write, size_t* size);
void MPARAM_OBJECT_TO_STR_C_ALLOC(mustache_param_object* p, char* write, size_t* size);

void MIDVLPARAM_TO_STR_C_ALLOC(void* vparam, char* write, size_t* size)  {
    char tmp[256];
    mustache_param* param = vparam;
    if (param->type == MUSTACHE_PARAM_NUMBER) {
        mustache_param_number *asnum = (mustache_param_number*)param;
        // element len
        *size+= sprintf_s(tmp,sizeof(tmp), "%f", asnum->value);
        if (write) {
            memcpy((char*)write,tmp,strlen(tmp));
        }
    }
    else if (param->type == MUSTACHE_PARAM_STRING) {
        mustache_param_string *as_str = (mustache_param_string*)param;
        if (write) {
            *(char*)write = '\"';
            memcpy(write+1, as_str->str.u, as_str->str.len);
            *(write+as_str->str.len+1) = '\"';
        }
        *size += 2 + as_str->str.len;
    }
    else if (param->type == MUSTACHE_PARAM_BOOLEAN) {
        mustache_param_boolean *as_str = (mustache_param_boolean*)param;
        if (as_str->type == 0) {
            *size += strlen("false");
            if (write) {
                memcpy(write, "false", strlen("false"));
            }
        } else {
            *size += strlen("true");
            if (write) {
                memcpy(write, "true", strlen("true"));
            }
        }
    }
    else if (param->type == MUSTACHE_PARAM_LIST) {
        MPARAM_LIST_TO_STR_C_ALLOC(vparam,write,size);
    }
    else if (param->type == MUSTACHE_PARAM_OBJECT) {
        MPARAM_OBJECT_TO_STR_C_ALLOC(vparam,write,size);
    }
}

void MPARAM_OBJECT_TO_STR_C_ALLOC(mustache_param_object* p, char* write, size_t* size) {
    if (write) {
        *(char*)write = '{';
    }

    size_t clen = 1;

    mustache_param* child = p->pMembers;
    while (child)
    {
        if (!write) {
            MIDVLPARAM_TO_STR_C_ALLOC(child, NULL, &clen);
        }
        if (write) {
            MIDVLPARAM_TO_STR_C_ALLOC(child, write+clen, &clen);
            (write)[clen] = ',';
        }
        clen+=1;
        child = child->pNext;
    }    

    if (write) {
        if (write[clen-1]=='{') {
            (write)[clen] = '}';
            clen++;
        } else {
            write[clen-1]='}';
        }
    }

    *size+=clen;
}

void MPARAM_LIST_TO_STR_C_ALLOC(mustache_param_list* p, char* write, size_t* size) {
    if (write) {
        *(char*)write = '[';
    }

    size_t clen = 1;

    size_t i = p->valueCount;
    mustache_param* child = p->pValues;
    while (child && i != 0)
    {
        if (!write) {
            MIDVLPARAM_TO_STR_C_ALLOC(child, NULL, &clen);
        }
        if (write) {
            MIDVLPARAM_TO_STR_C_ALLOC(child, write+clen, &clen);
            (write)[clen] = ',';
        }
        clen+=1;
        i--;
        child = child->pNext;
    }
    if (write) {
        if (write[clen-1]=='[') {
            (write)[clen] = ']';
            clen++;
        } else {
            write[clen-1]=']';
        }
    }

    *size+=clen;
}

mustache_slice MPARAM_TO_STR_C_ALLOC(void** bufptr, void* vparam) 
{
    char* buf = *bufptr;
    size_t add_len = 0;
    MIDVLPARAM_TO_STR_C_ALLOC(vparam, NULL, &add_len);

    size_t size = sizeof(size_t)+add_len;
    if (!buf) {
        buf = malloc(size);
        *(size_t*)buf = size;
    } else {
        size = *(size_t*)buf+add_len;
        buf = realloc(buf, size);
        *(size_t*)buf = size;
    }

    size_t scratch=0;
    MIDVLPARAM_TO_STR_C_ALLOC(vparam, (char*)buf + size-add_len, &scratch);

    *bufptr = buf;
    char* str = buf+sizeof(size_t);
    return (mustache_slice){.u = (char*)buf + size-add_len, add_len};
}