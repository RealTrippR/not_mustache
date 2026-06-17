const char* TEMPLATE =
"FOREACH EXAMPLE\n"
"foreach on lists\n"
"{{#list}}\n"
"--\n"
" + {{.}}\n"
"--"
"{{/}}\n";
// "and conditionals: {(str)}\n"


#include "example_common.h"



int main() 
{
    MUSTACHE_RES res = 0;

    const char *json1 = "{\n"
    "   \"people\": [\n"
    "       \"Dennis\":"
            " {\"age\": 20},\n"
    "       \"Aubrey\":"
            " {\"age\": 18},\n"
    "       \"Jonathan\":"
            " {\"age\": 19},\n"
    "       \"Noah\":"
            " {\"age\": 21},\n"
    "       \"Grace\":"
            " {\"age\": 20}\n"
    "   ]\n"
    "}";

    const char *json2 = "{\n"
    "   \"people\": [\n"
    "       {\"name\":\"Dennis\", \"age\": 20},"
    "       {\"name\":\"Aubrey\", \"age\": 18},"
    "       {\"name\":\"Jonathan\", \"age\": 19},"
    "       {\"name\":\"Noah\", \"age\": 21},"
    "       {\"name\":\"Grace\", \"age\": 20},"
    "   ]\n"
    "}";

    const char *json = json1;


    printf("JSON: \n%s", json);

    mustache_json_info json_info = {
        .copy_strings = 1,
        .use_parser_alloc_free = 0,
        .parser = &parser
    };


    mustache_param* json_first_param;
    printf("Converting JSON to mustache parameter chain... ");
    MUSTACHE_RES r = mustache_JSON(&parser, (mustache_const_slice){json,strlen(json)}, &json_info);
    if (r!=0) {
        printf("FAIL\n");
        return r;
    }
    printf("OK\n");
    /* the buffer size will be set the number of bytes
    needed to hold the parameters and any associated data 
    on the first call to mustache_json. */

    if (!json_info.use_parser_alloc_free) {
        if (json_info.buffer_size) 
            json_info.buffer = malloc(json_info.buffer_size);
            if (!json_info.buffer) {
                printf("FAIL: BAD ALLOCATION.\n");
                return MUSTACHE_ERR_ALLOC;
            }
            
            MUSTACHE_RES r = mustache_JSON(&parser, (mustache_const_slice){json,strlen(json)}, &json_info);

            if (r!=0) {
                printf("FAIL\n");
                return r;
            }
            printf("OK\n");
        }
    }

    mustache_JSON_free(parser, &json_info);


    return res;
}