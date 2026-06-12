const char* TEMPLATE =
"* * STRING TEMPLATE * *\n"
"/{{ The len() function evaluates the length\n"
"of a string parameter or the number of children\n"
"parameters within a list or object parameter.\n"
"}}\n"
"\n"
"For example, calling len on the object parameter {{&obj_as_str}}\n"
"evaluates as {{len(obj)}}\n"
"\n"
"len on the list parameter {{&list_as_str}}\n"
"evaluates as {{len(list)}}\n"
"\n"
"len on the string parameter {{&str_as_str}}\n"
"evaluates as {{len(str)}}\n"
"\n"
"not_mustache also allows for multiple variables to be enclosed, like so:"
"{{\n"
"&obj_as_str\n"
"len(str)\n"
"}}\n"
"and conditionals: {()}\n"
"{{/}}";



#include "example_common.h"

int main() 
{
    MUSTACHE_RES res = 0;

    char* parsed_template = NULL;
    size_t parsed_template_len;


    if ((res = parse_template(TEMPLATE, &parsed_template, &parsed_template_len, &listvar_astr))<0) {
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

    if (pstrbuf) {
        free(pstrbuf);
    }
    

    return res;
}