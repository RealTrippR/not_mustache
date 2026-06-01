const char* TEMPLATE =
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
"evaluates as {{len(str)}}\n";



#include <example_common.h>



int main() 
{
    MUSTACHE_RES res = 0;

    MPARAM_CSTR(str1, "", NULL,  "Am");
    MPARAM_CSTR(str2, "", &str1, "I");
    MPARAM_CSTR(str3, "", &str2, "Therefore");
    MPARAM_CSTR(str4, "", &str3, "Think");
    MPARAM_CSTR(str5, "", &str4, "I");


    MPARAM_OBJECT(objvar, "obj", NULL, &str5);
    MPARAM_CSTR(strvar, "str", &objvar, "Hello World.");
    MPARAM_LIST(listvar, "list", &strvar, 5, &str5);


    void *pstrbuf=NULL;

    MPARAM_STR(objvar_astr, "obj_as_str", &listvar, MPARAM_TO_STR_C_ALLOC(&pstrbuf, &objvar));
    MPARAM_STR(strvar_astr, "str_as_str", &objvar_astr, MPARAM_TO_STR_C_ALLOC(&pstrbuf, &strvar));
    MPARAM_STR(listvar_astr, "list_as_str", &strvar_astr, MPARAM_TO_STR_C_ALLOC(&pstrbuf, &listvar));


    char* parsed_template = NULL;
    size_t parsed_template_len;


    if ((res = parse_template(TEMPLATE, &parsed_template, &parsed_template_len, &listvar_astr))<0) {
        goto bail;
    }

    if (parsed_template_len == 0) {
        printf("PARSED TEMPLATE IS EMPTY [parsed_template_len = 0].\n");
    } else {
        printf("PARSED TEMPLATE\n=============================\n%.*s",parsed_template_len, parsed_template);
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