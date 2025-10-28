#include "mustache.h"
#include <string.h>
#include <streql/streqlasm.h>

#ifdef MUSTACHE_SYSTEM_TESTS
#include <stdio.h>
#endif // !MUSTACHE_SYSTEM_TESTS

#define min(X, Y) ((X) < (Y) ? (X) : (Y))
#define array_count(A) (sizeof(A)/sizeof(A[0]))

static uint32_t u32_round_to_next_power_of_2(uint32_t v) {
    v--; //https://stackoverflow.com/questions/466204/rounding-up-to-next-power-of-2
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}
void mustache_template_cache_init(mustache_template_cache* cache)
{
    memset(cache, 0, sizeof(mustache_template_cache));
}

bool mustache_check_cache(mustache_template_cache* cache, mustache_const_slice itemkey)
{
    return false;
}

static uint8_t mustache_cache_request_insertion_from_blocks(mustache_template_cache* cache, mustache_cache_lookup_block* prevFilledBlock,
    mustache_cache_lookup_block* nextFilledBlock, uint32_t insertionSize/*in bytes*/)
{

    uint32_t lowerBound = 0;
    uint32_t upperBound = cache->entryBuffer.len;
    lowerBound = prevFilledBlock->offset + prevFilledBlock->capacity;
    upperBound = nextFilledBlock->offset + nextFilledBlock->capacity;

    uint32_t dist = upperBound - lowerBound;

    return MUSTACHE_SUCCESS;
}

static uint8_t mustache_cache_request_block_resize(mustache_template_cache* cache, mustache_cache_lookup_block* blockToExpand, uint32_t newCapacity)
{
    // no lookup block exists for this item
    // if no lookup block exists, find available space
    const uint8_t firstByte = blockToExpand - cache->firstByteLookup;
    // find the previous block which does not have have an capacity of 0
    mustache_cache_lookup_block* prevFilledBlock = cache->firstByteLookup + firstByte;
    mustache_cache_lookup_block* nextFilledBlock = cache->firstByteLookup + firstByte;

    while (prevFilledBlock != cache->firstByteLookup)
    {
        prevFilledBlock--;
        if (prevFilledBlock->capacity > 0) {
            break;
        }
    }

    while (nextFilledBlock != cache->firstByteLookup + array_count(cache->firstByteLookup) - 1)
    {
        nextFilledBlock++;
        if (nextFilledBlock->capacity > 0) {
            break;
        }
    }

    uint32_t lowerBound = 0;
    uint32_t upperBound = cache->entryBuffer.len;

    if (prevFilledBlock->capacity > 0) {
        lowerBound = prevFilledBlock->offset + prevFilledBlock->capacity;
    }
    if (nextFilledBlock->capacity > 0) {
        upperBound = nextFilledBlock->offset + nextFilledBlock->capacity;
    }

    uint32_t availableCapacity = upperBound - lowerBound;
    if (newCapacity <= availableCapacity) {
        blockToExpand->capacity = newCapacity;
        return MUSTACHE_SUCCESS;
    }
    else {
        if (nextFilledBlock->capacity == 0) {
            return MUSTACHE_ERR_NO_SPACE;
        }

        // first, get the last block that will allow a shift with enough distance
        uint32_t distNeeded = newCapacity - blockToExpand->capacity;
        uint32_t distAvailable=0;
        mustache_cache_lookup_block* last;
        uint16_t b;
        for (b = firstByte; b < UINT8_MAX; ++b)
        {
            mustache_cache_lookup_block* block = cache->firstByteLookup + b;
            mustache_cache_lookup_block* next = cache->firstByteLookup + b;
            while (next != cache->firstByteLookup + array_count(cache->firstByteLookup) - 1)
            {
                next++;
                if (next->capacity > 0) {
                    break;
                }
            }
            b = next - cache->firstByteLookup;
            
            distAvailable += (next->offset) - (block->offset + block->capacity);
            if (distAvailable > distNeeded) {
                break;
            }
        }
    }
    return MUSTACHE_ERR_NO_SPACE;
}

uint8_t mustache_cache_set_item(mustache_template_cache* cache, mustache_const_slice itemkey, mustache_cache_item item)
{
    if (itemkey.len==0)
        return MUSTACHE_ERR;
            
    char firstbyte = itemkey.u[0];
    if (cache->firstByteLookup[firstbyte].capacity>0)
    {
        // a lookup block already exists for this item
        mustache_cache_lookup_block* block = &cache->firstByteLookup[firstbyte];
        
        // check if an entry exists in the block
        
        // write block
        mustache_cache_entry* entries = cache->entryBuffer.u + block->offset;
        for (uint32_t ei = 0; ei < block->entryCount; ++ei)
        {
            if (strneql(entries[ei].key.u, itemkey.u, min(itemkey.len, entries[ei].key.len))) {
                // entry already exists, overwrite it
                entries[ei].item = item;
                return MUSTACHE_SUCCESS;
            }
        }
        if (block->capacity/sizeof(mustache_cache_entry) < (block->entryCount+1)*sizeof(mustache_cache_entry)) {
            // attempt expansion
            uint32_t neededCapacity = u32_round_to_next_power_of_2(block->capacity + sizeof(mustache_cache_entry));

            uint8_t err = mustache_cache_request_block_resize(cache, block, neededCapacity);
            if (err != 0) {
                return err;
            }

            mustache_cache_entry* entries = cache->entryBuffer.u + block->offset;
            entries[block->entryCount].item = item;
            entries[block->entryCount].key = itemkey;
            block->entryCount++;
        }
        else {
            mustache_cache_entry* entries = cache->entryBuffer.u + block->offset;
            entries[block->entryCount].item = item;
            entries[block->entryCount].key = itemkey;
            block->entryCount++;
        }

    } else {
        // no lookup block exists for this item
        // if no lookup block exists, find available space
        mustache_cache_lookup_block* block = &cache->firstByteLookup[firstbyte];

        // find the previous block which does not have have an capacity of 0
        mustache_cache_lookup_block* prevFilledBlock = cache->firstByteLookup + firstbyte;
        mustache_cache_lookup_block* nextFilledBlock = cache->firstByteLookup + firstbyte;
        while (prevFilledBlock != cache->firstByteLookup)
        {
            prevFilledBlock--;
            if (prevFilledBlock->capacity > 0) {
                break;
            }
        }

        while (nextFilledBlock != cache->firstByteLookup + array_count(cache->firstByteLookup) - 1)
        {
            nextFilledBlock++;
            if (nextFilledBlock->capacity > 0) {
                break;
            }
        }

        uint32_t lowerBound = 0;
        uint32_t upperBound = cache->entryBuffer.len;

        if (prevFilledBlock->capacity > 0) {
            lowerBound = prevFilledBlock->offset + prevFilledBlock->capacity;
        }
        if (nextFilledBlock->capacity > 0) {
            upperBound = nextFilledBlock->offset + nextFilledBlock->capacity;
        }

        uint32_t dist = upperBound-lowerBound;
        if (sizeof(mustache_cache_entry) < dist)  // we have enough space for an entry

        {
            block->offset = lowerBound;
            block->capacity = u32_round_to_next_power_of_2(sizeof(mustache_cache_entry));
            block->entryCount = 1;
            mustache_cache_entry* entries = cache->entryBuffer.u + block->offset;
            entries[0].item = item;
            entries[0].key = itemkey;
            return MUSTACHE_SUCCESS;
        }
        else {
            // not enough space, request entry
            mustache_cache_request_insertion_from_blocks(cache, prevFilledBlock, nextFilledBlock, sizeof(mustache_cache_entry));
        }
        int i=0;
    }
    return MUSTACHE_ERR;
}

uint8_t mustache_cache_defragment(mustache_template_cache* cache)
{
    return MUSTACHE_SUCCESS;
}

uint8_t mustache_parse_file(mustache_parser* parser, mustache_const_slice filename,
     mustache_template_cache* cache, mustache_param_list* params)
{   

    return MUSTACHE_SUCCESS;
}









#ifdef MUSTACHE_SYSTEM_TESTS
void mustache_cache_print_first_byte_lookup(const mustache_template_cache* cache)
{
    printf("\nCACHE can hold a maximum of %d lookup entries.\n", cache->entryBuffer.len / sizeof(mustache_cache_entry));
    for (uint32_t i = 0; i < 256; ++i) {
        const mustache_cache_lookup_block* block = cache->firstByteLookup + i;
        if (block->capacity > 0) {
            printf("==== FIRST BYTE LOOKUP BLOCK #%d ====\n", i);
            printf("\tCAPACITY: %d\n", block->capacity);
            printf("\tOFFSET: %d\n", block->offset);
        }
    }
}


void mustache_cache_print_entries_of_lookup_block(const mustache_template_cache* cache, uint8_t byte)
{
    uint32_t i = 0;
    mustache_cache_entry* entry = cache->entryBuffer.u + cache->firstByteLookup[byte].offset;
    printf("==== BLOCK '%c'(#%d) ENTRY COUNT: %d ====\n", byte, byte, cache->firstByteLookup[byte].entryCount);
    for (uint8_t i = 0; i < cache->firstByteLookup[byte].entryCount; ++i) 
    {
        printf("\t==== CACHE ENTRY #%d ====\n",i);
        printf("\t\tNAME: %.*s\n", entry->key.len,entry->key.u);
        printf("\t\tDATA LEN.: %d\n", entry->item.dataLen);

        entry++;
    }
}

#endif