#define MUSTACHE_SYSTEM_TESTS


#include <mustache.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

void parse_callback(mustache_parser* parser, void* udata, mustache_slice parsed)
{

    return;
}

int main()
{
    uint8_t ITEM_BUFFER[16384];
    uint8_t PARSER_BUFFER[32768];
    mustache_parser parser;


    mustache_template_cache templateCache;
    mustache_template_cache_init(&templateCache);

   // templateCache.lookupBuffer = (mustache_slice){TEMPLATE_BUFFER, sizeof(TEMPLATE_BUFFER)};
    templateCache.entryBuffer = (mustache_slice){ITEM_BUFFER, sizeof(ITEM_BUFFER)};
    mustache_param_list paramlist;
    

    //mustache_load_param_list_from_file(&paramlist, "comments.json");
    const char* filename = "basic.html";
    mustache_const_slice filenameSlice = { filename, strlen(filename) };

    const char* filename2 = "basic2.html";
    mustache_const_slice filenameSlice2 = { filename2, strlen(filename2) };

    const char* filename3 = "basic3.html";
    mustache_const_slice filenameSlice3 = { filename3, strlen(filename3) };

    const char* filename4 = "apples.html";
    mustache_const_slice filenameSlice4 = { filename4, strlen(filename4) };

    const char* filename5 = "carbon.html";
    mustache_const_slice filenameSlice5 = { filename5, strlen(filename5) };

    mustache_cache_item item;
    item.data = "Hi";
    item.dataLen = strlen(item.data);
    MUSTACHE_RES err = mustache_cache_set_item(&templateCache, filenameSlice, item);

    mustache_cache_item item4;
    item4.data = "Apples & fruits";
    item4.dataLen = strlen(item4.data);
    err = mustache_cache_set_item(&templateCache, filenameSlice4, item4);

    mustache_cache_item item2;
    item2.data = "Hello";
    item2.dataLen = strlen(item2.data);
    err = mustache_cache_set_item(&templateCache, filenameSlice2, item2);

    mustache_cache_item item5;
    item5.data = "Carbon is a derivative of the C++ language with an emphasis on memory safety and improved syntax.";
    item5.dataLen = strlen(item5.data);
    err = mustache_cache_set_item(&templateCache, filenameSlice5, item5);

    mustache_cache_item item3;
    item3.data = "My name is <.....>";
    item3.dataLen = strlen(item3.data);
    err = mustache_cache_set_item(&templateCache, filenameSlice3, item3);

    if (mustache_parse_file(&parser, filenameSlice, &paramlist, 
        (mustache_slice){ PARSER_BUFFER,sizeof(PARSER_BUFFER) }, 
        NULL, parse_callback) != MUSTACHE_SUCCESS) 
    {
        printf("MUSTACHE: FAILED TO PARSE FILE");
    }

   // mustache_cache_remove_item(&templateCache, filenameSlice3);
    
    mustache_cache_print_first_byte_lookup(&templateCache);

    mustache_cache_print_entries(&templateCache);

    mustache_cache_validate(&templateCache);

    return 0;
}