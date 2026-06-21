#include "example_common.h"



int main() 
{
    MUSTACHE_RES res = 0;

    const char *json1 = "{\n"
    "   \"people\": [\n"
    "       \"Dennis\":"
            " {\"age\": 20},\n"
    "       \"Audrey\":"
            " {\"age\": 18},\n"
    "       \"Jonathan\":"
            " {\"age\": 19},\n"
    "       \"Noah\":"
            " {\"age\": 21},\n"
    "       \"Grace\":"
            " {\"age\": 20}\n"
    "   ]\n"
    "}\0JSON_END";

    const char *json2 = "{\n"
    "   \"people\": [\n"
    // "       \"Dennis\",\n"
    // "       \"Audrey\",\n"
    // "       \"Noah\",\n"
    // "       \"Jonathan\",\n"
    // "       \"Grace\"\n"
    "       {\"name\":\"Dennis\", \"age\": 20},\n"
    "       {\"name\":\"Audrey\", \"age\": 18},\n"
    "       {\"name\":\"Jonathan\", \"age\": 19},\n"
    "       {\"name\":\"Noah\", \"age\": 21},\n"
    "       {\"name\":\"Grace\", \"age\": 20},\n"
    "   ]\n"
    "}\0JSON_END";


    const char* json3 = "{\n"
    "  \"waterlevels_in\": [\n"
    "       -5.25,"
    "       -2.0005,"
    "       -3.1345,"
    "       0.00000,"
    "       2.9575,"
    "       4.750,"
    "       2,"
    "       10,"
    "       12,"
    "       0"
    "   ],"
    "   \"waterlevels_um\": ["
    "       -2325.3253,"
    "       -9895398.53053,"
    "       -21336363.55,"
    "       230603463.366,"
    "       10.25e10,"
    "       10.25E10,"
    "       23985e-20,"
    "       320959e-11"
    "   ]"   
    "}\0JSON_END";


    const char *json = json2;


    printf("JSON: \n%s\n\n", json);

    mustache_json_info json_info = {
        .copy_strings = 1,
        .use_parser_alloc_free = 0,
        .parser = &parser,
        .numinfo = {
            .max_decimals = 15,
            .trim_zeros = 0
        }
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
        if (json_info.buffer_size) {
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


    mustache_print_parameter_list(json_info.first_param);

    mustache_JSON_free(&parser, &json_info);

    return res;
}