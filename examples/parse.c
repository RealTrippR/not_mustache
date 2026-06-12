const char* TEMPLATE =/*
"Not mustache has no strict requirements for the design of templating engines.\n"
"The engine included with this project uses a buffered callback system, in which\n"
"a stream object (mustache_stream) copies data from a stream into the input buffer\n"
"fed as an argument into mustache_parse_stream or mustache_parse_file.\n"
"Note that streams must be deterministic, and the behavior or properties of a stream change in any"
"way, the mustache_structure chain that was built with it must be destroyed with mustache_structure_chain_free"
"These input chunks are processed by the template engine and fed into the parseCallback argument of\n"
"mustache_parse_stream or mustache_parse_file. This processed data will be stored in the output buffer.\n"
"\n"
"The mustache_structure is an opaque object type used to cache the structure of template. It improves performance\n"
"by converting the template in a binary format instead of re-parsing the source template every call.\n"
"To generate this structure, memory is allocated with parser_alloc and parser_free. It is automatically\n"
"generated on its first usage and will be reused in every subsequent parse.\n"
"Call structure_chain_flush to clear a structure chain if any of the following used to generate it\n"
"have changed:\n"
"- parameter addresses\n"
"- parameter types\n"
"\n"
"A structure chain must be destroyed with mustache_structure_chain_free if the contents of the source template have changed.\n"
"\n"
"mustache_structure_chain_free MUST BE CALLED to free any memory allocated for a mustache_structure chain."
"\n"
"\n"
"Note that mustache_parse_file uses mustac"*/"he_parse_stream internally, it only exists to\n"
"simplify the process of reading template files.\n"
"\n"
"\n"
"All chunks fed to this engine from a stream must contain only complete top-level scopes,\n"
"they cannot be split across chunks.\n"
"For example, /{{#mybool}} {{var}} {{/}} is valid, "
"but /{{#mybool}} {{var}} in one chunk and {{/}} in next is invalid.\n"
"\n"
"\n"
"This parsing engine does not do any significant checks on the validity of a not-mustache template;\n"
"it is not recommended that you use untrusted templates in this parsing engine."
"\n"
"The template below is identical to the one in string.c, but it will be fed\n"
"into a parser that has a 256 byte input buffer, much smaller than the one used in string.c.\n"
"This forces the parser to handle the template in multiple chunks.";
/*
"\n"
"\n"
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
"evaluates as {{len(str)}}\n";
*/

#include "example_common.h"



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


    if ((res = parse_template_segmented(TEMPLATE, &parsed_template, &parsed_template_len, &listvar_astr))<0) {
        printf("Error parsing template.\n");
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

    

    return res;
}