#include <stdlib.h>
#include <stdio.h>

#include <not_mustache/not_mustache.h>
#include <not_mustache/not_mustache_pmacros.h>

#define SEGMENT_INPUT_BUFFER_LEN 256

uint8_t PARSER_STRUCTURE_BUFFER[65536];
typedef struct 
{
    void* block;
    size_t size; // used size
    size_t capacity; // total capacity
    char* parsed_data; // ONLY USE FOR parse_template_segmented, DO NOT USE THIS IN ANY OTHER CIRCUMSTANCES,
                        //  char** parsed_template, will be set to this value
    size_t parsed_data_len;
} parser_udata;
parser_udata parserudata = { .block = PARSER_STRUCTURE_BUFFER, .size = 0, .capacity = sizeof(PARSER_STRUCTURE_BUFFER)};


typedef struct
{
    const unsigned char* data;  // ptr to template source 
    size_t cur;                 // offset from template source
    size_t len;                 // total length of template source
} stream_udata;
stream_udata streamudata;


void* parser_alloc(mustache_parser* parser, size_t bytes) {
    parser_udata* udata = parser->userData;
    if (udata->size + bytes > udata->capacity) {
        return NULL;
    }
    uint8_t*p= (uint8_t*)udata->block + udata->size;
    udata->size += bytes;
    return p;
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
    udata->cur += cpylen;
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



void parser_err_callback(mustache_parser* err, char msg[256], const char* src, const char* src_end) {
    printf("\nPARSER ERROR\n============\n%s\n", msg);
    if (src) {
        size_t len = (src_end-src < 32) ?  src_end-src : 32;
        printf("LINE+32: %.*\n", len, src);
    }
}



void parser_warn_callback(mustache_parser* err, char msg[256], const char* src, const char* src_end) {
    printf("\nPARSER WARNING\n============\n%s\n", msg);
    if (src) {
    size_t len = (src_end-src < 32) ?  src_end-src : 32;
        printf("LINE+32: %.*\n", len, src);
    }
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
    uint8_t PARSER_BUFFER_PADDING[1024]; // THIS ONLY EXISTS FOR DEBUGGING PURPOSES, IT CAN SAFELY BE REMOVED
    memset(PARSER_BUFFER_PADDING, 0xFF,sizeof(PARSER_BUFFER_PADDING));
    uint8_t PARSER_OUTPUT_BUFFER[2*8192]; // this buffer will be populated on calls to parser.alloc
    uint8_t PARENT_STACK_BUFFER[2048];


    streamudata = (stream_udata){ .data = template_source, .cur = 0, .len = strlen(template_source) };

    parser_stream.readCallback = stream_read_callback;
    parser_stream.seekCallback = stream_seek_callback;
    parser.err_callback = parser_err_callback;
    parser.warn_callback = parser_warn_callback;

    *parsed_template_length=0;
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



void write_parsed_data_callback(mustache_parser* parser, void* __udata, mustache_slice parsed) {
    parser_udata* udata = __udata;
    void* tmp = realloc(udata->parsed_data,udata->parsed_data_len+parsed.len);
    if (!tmp) {
        return;
    }
    udata->parsed_data = tmp;
    memcpy((char*)udata->parsed_data+udata->parsed_data_len,parsed.u,parsed.len);
    udata->parsed_data_len+=parsed.len;
    
   // printf("\nwrite_parsed_data_callback: %d bytes added to output.\n=====\n%.*s\n====\n\n", parsed.len, parsed.len,parsed.u);

}




size_t segmented_stream_read_callback(void* u, uint8_t* dst, size_t dstlen) {
    stream_udata* udata = u;

    const char *f,*e; 
    if (mustache_get_stream_range(
        SEGMENT_INPUT_BUFFER_LEN,
        &f,
        &e,
        udata->data+udata->cur,
        udata->data+udata->len
    ) <0) {
        printf("segmented_stream_read_callback: closing stream, unable to fit complete mustache range within input buffer.\n");
        return 0;
    }

    size_t cpylen = e-f;
    if (cpylen > dstlen) {
        cpylen = dstlen;
    }

    memcpy(dst, udata->data + udata->cur, cpylen);
    printf("COPYING SLICE [%d bytes] =======\n%.*s\n",e-f,cpylen, udata->data+udata->cur);

    udata->cur += cpylen;

    return cpylen;
}

uint64_t segmented_stream_seek_callback(void* u, int64_t whence, MUSTACHE_SEEK_DIR seekdir) {
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



/* identical to parse_template, except it uses a much, much smaller input and output buffer, forcing the parser to 
parse it across several chunks*/
MUSTACHE_RES parse_template_segmented(const char* template_source, char** parsed_template, size_t* parsed_template_length, void* arglist) {
    uint8_t PARSER_INPUT_BUFFER[SEGMENT_INPUT_BUFFER_LEN];
    uint8_t PARSER_OUTPUT_BUFFER[SEGMENT_INPUT_BUFFER_LEN*4];
    uint8_t PARENT_STACK_BUFFER[1024];

    streamudata = (stream_udata){ .data = template_source, .cur = 0, .len = strlen(template_source) };

    parser_stream.readCallback = segmented_stream_read_callback;
    parser_stream.seekCallback = segmented_stream_seek_callback;
    parser.err_callback = parser_err_callback;
    parser.warn_callback = parser_warn_callback;

    *parsed_template = NULL;

    

    MUSTACHE_RES m = mustache_parse_stream(
        &parser,
        (mustache_slice){PARENT_STACK_BUFFER, sizeof(PARENT_STACK_BUFFER)},
        &parser_stream,
        &struct_chain,
        arglist,
        (mustache_slice){PARSER_INPUT_BUFFER, sizeof(PARSER_INPUT_BUFFER)},
        (mustache_slice){PARSER_OUTPUT_BUFFER, sizeof(PARSER_OUTPUT_BUFFER)},
        &parserudata,
        write_parsed_data_callback
    );

    *parsed_template_length = parserudata.parsed_data_len;
    *parsed_template = parserudata.parsed_data;
    if (m<0) {
        return m;
    } else {
        if (*parsed_template_length == 0) {
            return MUSTACHE_SUCCESS;
        }

        
        return m;
    }
}





MUSTACHE_RES free_template(char* parsed_template)
{
    printf("\ntodo: implement mustache_structure_chain_free\n");
    //mustache_structure_chain_free(&parser, &struct_chain);
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