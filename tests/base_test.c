#define MUSTACHE_SYSTEM_TESTS


#include <mustache.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

void on_cache_entry_expiry(mustache_template_cache* cache, mustache_cache_entry* entry)
{
    printf("Cache entry of key: %.*s ... has expired. \n", entry->key.len, entry->key.u);
}

int main()
{
    uint8_t TEMPLATE_BUFFER[128];
    uint8_t ITEM_BUFFER[32384];
    uint8_t PARSER_BUFFER[32384];
    mustache_parser parser;
    parser.buffer = (mustache_slice){PARSER_BUFFER, sizeof(PARSER_BUFFER)};


    mustache_template_cache templateCache;
    mustache_template_cache_init(&templateCache);

   // templateCache.lookupBuffer = (mustache_slice){TEMPLATE_BUFFER, sizeof(TEMPLATE_BUFFER)};
    templateCache.entryBuffer = (mustache_slice){ITEM_BUFFER, sizeof(ITEM_BUFFER)};
    templateCache.entryExpiryCallback = on_cache_entry_expiry;
    mustache_param_list paramlist;
    

    //mustache_load_param_list_from_file(&paramlist, "comments.json");
    const char* filename = "basic.html";
    mustache_const_slice filenameSlice = { filename, strlen(filename) };
    const char* filename2 = "basic2.html";
    mustache_const_slice filenameSlice2 = { filename2, strlen(filename2) };
    const char* filename3 = "basic3.html";
    mustache_const_slice filenameSlice3 = { filename3, strlen(filename3) };

    mustache_cache_item item;
    item.data = "Hi";
    item.dataLen = strlen(item.data);
    mustache_cache_set_item(&templateCache, filenameSlice, item);

    mustache_cache_item item2;
    item2.data = "Hello";
    item2.dataLen = strlen(item2.data);
    mustache_cache_set_item(&templateCache, filenameSlice2, item2);

    mustache_cache_item item3;
    item3.data = "My name is <.....>";
    item3.dataLen = strlen(item3.data);
    mustache_cache_set_item(&templateCache, filenameSlice3, item3);


    if (mustache_parse_file(&parser, filenameSlice, &templateCache, &paramlist)!=MUSTACHE_SUCCESS) {
        printf("MUSTACHE: FAILED TO PARSE FILE");
    }

    mustache_cache_defragment(&templateCache);


    mustache_cache_print_first_byte_lookup(&templateCache);

    mustache_cache_print_entries_of_lookup_block(&templateCache, 'b');

    return 0;
}