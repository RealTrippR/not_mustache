const char* TEMPLATE =
"FOREACH EXAMPLE\n"
"foreach on lists\n"
"{{#people}}\n"
"--\n"
//" + {{.name}}\n"
//" + {{.age}}\n"
" + {{.info.address}}\n"
"--"
"{{/}}"

"Foreach loops can only be used on parent parameters, of which there are two types: list and object.\n"
"Children can be indexed with bracket notation '[x]', where x is an integer, the first index starting at 0."
"If the value is negative, it will be an backwards offset beginning at the last element of the array, starting at -1. (i.e. [-1] would be the last element, [-2] is second to last, etc.)\n"
"The list and object types are near-identical to each other, the only difference being that objects can be indexed by a constant"
"string, or dot notation.";

//"The first person is: {{people[0]}}";

#include "example_common.h"



int main() 
{
    MUSTACHE_RES res = 0;


    const char *json = "{\n"
    "   \"people\": [\n"
    "       {\"name\":\"Dennis\", \"age\": 20, \"hobbies\": [\"hiking\",\"programming\",\"guitar\",\"CS2 -_-\"], \"info\": {\"address\": \"123 main street.\"} },\n"
    "       {\"name\":\"Audrey\", \"age\": 18, \"hobbies\": [\"knitting\",\"cross-country\"], \"info\": {\"address\": \"124 main street.\"} },\n"
    "       {\"name\":\"Jonathan\", \"age\": 19, \"hobbies\": [\"art\",\"bmx\",\"football\"], \"info\": {\"address\": \"125 main street.\"} },\n"
    "       {\"name\":\"Noah\", \"age\": 21, \"hobbies\": [\"cooking\"], \"info\": {\"address\": \"126 main street.\"} },\n"
    "       {\"name\":\"Grace\", \"age\": 20, \"hobbies\": [\"sketching\", \"piano\"], \"info\": {\"address\": \"127 main street.\"} }\n"
    "   ]\n"
    "}\0JSON_END";


    mustache_json_info json_info = {
        .copy_strings = true,
        .use_parser_alloc_free = true,
        .parser = &parser,
        .numinfo = {
            .max_decimals = 15,
            .trim_zeros = false
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

    mustache_print_parameter_list(json_info.first_param);


    const mustache_param_list *people = ((mustache_param_object*)json_info.first_param)->pMembers;

    

    char* parsed_template = NULL;
    size_t parsed_template_len;



    if ((res = parse_template(TEMPLATE, &parsed_template, &parsed_template_len, people))<0) {
        goto bail;
    }

    if (parsed_template_len == 0) {
        printf("PARSED TEMPLATE IS EMPTY [parsed_template_len = 0].\n");
    } else {
        printf("PARSED TEMPLATE\n=============================\n%.*s",(uint32_t)parsed_template_len, parsed_template);
    }

bail:
    if (parsed_template) {
        free_template(parsed_template);
        parsed_template = NULL;
    }
    

    mustache_JSON_free(&parser, &json_info);

    return res;
}