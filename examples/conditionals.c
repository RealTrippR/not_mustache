const char* TEMPLATE =
"not-mustache has support for conditional operators. These";


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