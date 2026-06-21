const char* TEMPLATE =
"not-mustache has support for conditional operators. These allow branching"
"within templates."
"For example: "
"{(people[0])}"
"   The first person '{(people[0].name)}' exists"
"{(/)}"


"Conditionals may be chained, involve multiple operators, and perform checks against variable states: "


"";

#include "example_common.h"

int main() 
{
    MUSTACHE_RES res = 0;


    const char *json = "{\n"
    "   \"people\": [\n"
    "       {\"name\":\"Dennis\", \"age\": 20, \"hobbies\": [\"hiking\",\"programming\",\"guitar\",\"CS2 -_-\"], \"info\": {\"address\": \"123 main street.\"} },\n"
    "       {\"name\":\"Audrey\", \"age\": 18, \"hobbies\": [\"knitting\",\"cross-country\"]},\n"
    "       {\"name\":\"Jonathan\", \"age\": 19, \"hobbies\": [\"art\",\"bmx\",\"football\"]},\n"
    "       {\"name\":\"Noah\", \"age\": 21, \"hobbies\": [\"cooking\"]},\n"
    "       {\"name\":\"Grace\", \"age\": 20, \"hobbies\": [\"sketching\", \"piano\"]},\n"
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