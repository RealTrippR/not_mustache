/***************************************************
MIT License

Copyright (C) 2025 Tripp R

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
***************************************************/

#include "mustache.h"
#include <string.h>
#include <streql/streqlasm.h>
#include <math.h>

#ifndef NDEBUG
#include <assert.h>
#endif // !NDEBUG

#ifdef MUSTACHE_SYSTEM_TESTS
#include <stdio.h>
#endif // !MUSTACHE_SYSTEM_TESTS

#define min(X, Y) ((X) < (Y) ? (X) : (Y))
#define array_count(A) (sizeof(A)/sizeof(A[0]))

typedef struct {
    mustache_slice buf;
    uint32_t count;
    uint32_t MAX_COUNT;
} parent_stack;

static void parent_stack_pop(parent_stack* stack, void* param) 
{
#ifndef NDEBUG
    if (stack->count == 0) {
        assert(00 && "parent_stack_pop: INVALID CALL, STACK WILL UNDERFLOW!");
    }
#endif // !NDEBUG

    stack->count--;
}

static uint8_t parent_stack_push(parent_stack* stack, void* param)
{
#ifndef NDEBUG
    mustache_param* p = param;
    if (!(p->type == MUSTACHE_PARAM_LIST || p->type == MUSTACHE_PARAM_OBJECT)) {
        assert(00 && "parent_stack_push: param IS NOT A PARENT.");
    }
#endif // !NDEBUG

    if (stack->count == stack->MAX_COUNT) {
        return MUSTACHE_ERR_OVERFLOW;
    }

    uint8_t** pointers = (uint8_t**)stack->buf.u;
    pointers[stack->count] = (uint8_t*)param;
    stack->count++;

    return MUSTACHE_SUCCESS;
};



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

uint8_t digits_i64(int64_t n) {
    if (n < 10) return 1;
    if (n < 100) return 2;
    if (n < 1000) return 3;
    if (n < 10000) return 4;
    if (n < 100000) return 5;
    if (n < 1000000) return 6;
    if (n < 10000000) return 7;
    if (n < 100000000) return 8;
    if (n < 1000000000) return 9;
    if (n < 10000000000) return 10;
    if (n < 100000000000) return 11;
    if (n < 1000000000000) return 12;
    if (n < 10000000000000) return 13;
    if (n < 100000000000000) return 14;
    if (n < 1000000000000000) return 15;
    if (n < 10000000000000000) return 16;
    if (n < 100000000000000000) return 17;
    if (n < 1000000000000000000) return 18;
    if (n < 10000000000000000000) return 19;
    return 0;
}

static int16_t i64toa(int64_t n,uint8_t* buf, size_t size)
{
    int8_t dig = digits_i64(n);
    int16_t i = 0;
    while (n!=0)
    {
        i++;
        buf[dig - i] = n % 10 + 48;
        n /= 10;
    }
    return i;
}

// returns the expected write length of a dtoa call
static uint16_t dtoalen(double value, uint16_t precision, bool trimZeros) {
    if (isnan(value)) {
        return 3;
    }
    if (isinf(value)) {
        if (value < 0) {
            return 4;
        }
        else {
            return 3;
        }
    }

    if (value < 0) {
        value = -value;
    }

    uint16_t len = 0;
    if (value < 0) {
        len++;
        value = -value;
    }

    long long int_part = (long long)value;
    double frac_part = value - (double)int_part;

    if (int_part == 0) {
        len += 1;
    }
    else {
        long long tmp = int_part;
        while (tmp > 0) {
            len++;
            tmp /= 10;
        }
    }

    if (precision > 0)
    {
        if (!trimZeros)
        {
            len += 1 + precision; 
        }
        else {
            uint16_t frac_len = 0;
            double frac = frac_part;
            for (uint16_t i = 0; i < precision; i++) 
            {
                frac *= 10.0;
                int digit = (int)frac;
                frac -= digit;
                if (digit != 0) {
                    frac_len = i + 1;
                }
            }
            if (frac_len > 0) {
                len += 1 + frac_len; // '.' 
            }
        }
    }

    return len;
}

// converts a double to string and returns the write head
static uint8_t* dtoa(double value, uint8_t* buf, size_t size, uint16_t precision, bool trimZeros) {
    if (size == 0) return buf;

    if (isnan(value)) {
        memcpy(buf, "nan", min(size, 3));
        return buf + min(size, 3);
    }
    if (isinf(value)) {
        if (value < 0) {
            memcpy(buf, "-inf", min(size, 4));
            return buf + min(size, 4);
        }
        else {
            memcpy(buf, "inf", min(size, 3));
            return buf + min(size, 3);
        }
    }

    char* start = buf;

    if (value < 0) {
        if (size <= 1) return buf;
        *buf++ = '-';
        size--;
        value = -value;
    }

    // get integer and frac parts
    long long int_part = (long long)value;
    double frac_part = value - (double)int_part;

    // Write integer part
    int written = i64toa(int_part, buf, size);
    if (written < 0 || written >= (int)size) return buf;

    buf += written;
    size -= written;

    char frac_buf[64];
    uint8_t frac_len = 0;

    frac_part += 0.5 * pow(10, -precision);

    for (int32_t i = 0; i < precision && frac_len + 1 < (int32_t)sizeof(frac_buf); i++) {
        frac_part *= 10.0;
        int32_t digit = (int32_t)frac_part;
        frac_buf[frac_len++] = '0' + digit;
        frac_part -= digit;
    }

    if (trimZeros) {
        while (frac_len > 0 && frac_buf[frac_len - 1] == '0')
            frac_len--;
    }

    // skip '.' if no frac. digits remain
    if (frac_len == 0)
        return buf;

    // '.'
    if (size < 2) return buf;
    *buf++ = '.';
    size--;

    // write fractional digits
    for (uint8_t i = 0; i < frac_len && size > 1; i++) {
        *buf++ = frac_buf[i];
        size--;
    }

    return buf;
}

void mustache_template_cache_init(mustache_cache* cache)
{
    memset(cache, 0, sizeof(mustache_cache));
}

bool mustache_check_cache(mustache_cache* cache, mustache_const_slice itemkey)
{
    return false;
}

static uint8_t mustache_cache_request_insertion_from_blocks(mustache_cache* cache, mustache_cache_lookup_block* prevFilledBlock,
    mustache_cache_lookup_block* nextFilledBlock, uint32_t insertionSize/*in bytes*/)
{

    uint32_t lowerBound = 0;
    uint32_t upperBound = (uint32_t)cache->entryBuffer.len;
    lowerBound = prevFilledBlock->offset + prevFilledBlock->capacity;
    upperBound = nextFilledBlock->offset;

    uint32_t dist = upperBound - lowerBound;

    return MUSTACHE_SUCCESS;
}

static uint8_t mustache_cache_request_block_resize(mustache_cache* cache, mustache_cache_lookup_block* blockToExpand, uint32_t newCapacity)
{
    // no lookup block exists for this item
    // if no lookup block exists, find available space
    const uint8_t firstByte = (uint8_t)(blockToExpand - cache->firstByteLookup);
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
    uint32_t upperBound = (uint32_t)cache->entryBuffer.len;

    if (prevFilledBlock->capacity > 0) {
        lowerBound = prevFilledBlock->offset + prevFilledBlock->capacity;
    }
    if (nextFilledBlock->capacity > 0) {
        upperBound = nextFilledBlock->offset;
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
        uint32_t distNeeded = (uint32_t)(newCapacity - blockToExpand->capacity);
        uint32_t distAvailable = 0;
        mustache_cache_lookup_block* last = NULL;
        int16_t b;

        if (blockToExpand->next) 
        {
            mustache_cache_lookup_block* next = blockToExpand;
            while (next->next)
            {
                next = blockToExpand->next;
                if (next->next == NULL) {
                    // if next is the last block in the list
                    distAvailable += (uint32_t)cache->entryBuffer.len - (next->offset + next->capacity);
                    last = next; // block is the last block with contents
                    break;
                }
                else {

                    // skip ahead if next has contents
                    b = (int16_t)(next - cache->firstByteLookup);

                    distAvailable += (next->next->offset) - (next->offset + next->capacity);
                    if (distAvailable > distNeeded) {
                        last = next;
                        break;
                    }
                }
            }
        } 
        else {
            distAvailable += (uint32_t)(cache->entryBuffer.len - (blockToExpand->offset + blockToExpand->capacity));
            last = blockToExpand; // block is the last block with contents
        }

        if (!last || distAvailable < distNeeded) {
            return MUSTACHE_ERR_NO_SPACE;
        }

        // there's enough space to resize
        // beginning from the last block downwards, move data forwards to make room for the needed capacity

        b = (int16_t)(last - cache->firstByteLookup);
        mustache_cache_lookup_block* prevFilled = last;
        for (; b >= 0; b--)
        {
            mustache_cache_lookup_block* block = cache->firstByteLookup + b;
            if (block == blockToExpand) {
                blockToExpand->capacity = newCapacity;
                return MUSTACHE_SUCCESS;
            }
            uint32_t distToShift;
            if (block == last) {
                // LAST BLOCK
                distToShift = distNeeded;
            }
            else {
                distToShift = 0;
                if (block->capacity > 0) {
                    distToShift = (prevFilled->offset) - block->offset + block->capacity;

                    prevFilled = block;
                }
            }
            // shift entries ahead
            if (distToShift > 0) {
                uint8_t* dst = cache->entryBuffer.u + block->offset + distToShift;
                uint8_t* src = cache->entryBuffer.u + block->offset;
                memmove(dst, src, block->capacity);
                char b2 = (char)(block - cache->firstByteLookup);
                block->offset += distToShift;
            }
        }
        blockToExpand->capacity = newCapacity;
        return MUSTACHE_SUCCESS;
    }
    return MUSTACHE_ERR_NO_SPACE;
}

static inline mustache_cache_lookup_block* 
get_next_block_with_contents(mustache_cache* cache, mustache_cache_lookup_block* block)
{
    mustache_cache_lookup_block* endBlock = (mustache_cache_lookup_block*)(cache->firstByteLookup + array_count(cache->firstByteLookup));
    block++;
    while (block < endBlock)
    {
        if (block->capacity > 0)
        {
            return block;
        }
        block++;
    }

    return NULL;
}

static inline void
update_next_of_previous_block(mustache_cache* cache, mustache_cache_lookup_block* block)
{
#ifndef NDEBUG
    if (block->capacity == 0) {
        assert(00 && "update_next_of_previous_block MUST ONLY BE CALLED WITH A BLOCK OF A NON-ZERO CAPACITY");
    }
#endif // !NDEBUG

    mustache_cache_lookup_block* cur = block;
    while (cur != cache->firstByteLookup)
    {
        cur--;
        if (cur->capacity) {
            cur->next = block;
            return;
        }

    }
    return;
}

uint8_t mustache_cache_set_item(mustache_cache* cache, mustache_const_slice itemkey, mustache_cache_item item)
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
        mustache_cache_entry* entries = (mustache_cache_entry*)(cache->entryBuffer.u + block->offset);
        for (uint32_t ei = 0; ei < block->entryCount; ++ei)
        {
            if (itemkey.len == entries[ei].key.len && strneql(entries[ei].key.u, itemkey.u, itemkey.len)) {
                // entry already exists, overwrite it
                entries[ei].item = item;
                return MUSTACHE_SUCCESS;
            }
        }
        if (block->capacity/sizeof(mustache_cache_entry) < (block->entryCount+1)*sizeof(mustache_cache_entry)) {
            // attempt expansion
            uint32_t neededCapacity = u32_round_to_next_power_of_2((block->entryCount+1)* sizeof(mustache_cache_entry));

            uint8_t err = mustache_cache_request_block_resize(cache, block, neededCapacity);
            if (err) {
                return err;
            }

            mustache_cache_entry* entries = (mustache_cache_entry*)(cache->entryBuffer.u + block->offset);
            entries[block->entryCount].item = item;
            entries[block->entryCount].key = itemkey;
            block->entryCount++;
        }
        else {
            mustache_cache_entry* entries = (mustache_cache_entry*)(cache->entryBuffer.u + block->offset);
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
            upperBound = nextFilledBlock->offset;
        }

        uint32_t dist = upperBound-lowerBound;
        if (sizeof(mustache_cache_entry) <= dist)  // we have enough space for an entry

        {
            block->next = get_next_block_with_contents(cache, block);
            block->offset = lowerBound;
            block->capacity = u32_round_to_next_power_of_2(sizeof(mustache_cache_entry));
            block->entryCount = 1;
            update_next_of_previous_block(cache, block);

            mustache_cache_entry* entries = (mustache_cache_entry*)(cache->entryBuffer.u + block->offset);
            entries[0].item = item;
            entries[0].key = itemkey;
            return MUSTACHE_SUCCESS;
        }
        else {
            block->next = get_next_block_with_contents(cache, block);

            // not enough space, request entry
            const uint32_t neededCapacity = u32_round_to_next_power_of_2(block->capacity + sizeof(mustache_cache_entry));
            MUSTACHE_RES err = mustache_cache_request_block_resize(cache, block, neededCapacity);
            if (err) {
                block->next = NULL;
                return err;
            }
            block->offset = (uint32_t)lowerBound;
            block->capacity = neededCapacity;
            block->entryCount = 1;
            update_next_of_previous_block(cache, block);

            mustache_cache_entry* entries = (mustache_cache_entry*)(cache->entryBuffer.u + block->offset);
            entries[0].item = item;
            entries[0].key = itemkey;

            return err;
        }
        int i=0;
    }
    return MUSTACHE_ERR;
}

uint8_t mustache_cache_remove_item(mustache_cache* cache, mustache_const_slice itemKey)
{
    if (itemKey.len == 0) {
        return MUSTACHE_SUCCESS;
    }

    mustache_cache_lookup_block* block = cache->firstByteLookup + itemKey.u[0];
    if (block->capacity == 0) {
        return MUSTACHE_ERR_NONEXISTENT;
    }

    mustache_cache_entry* entry = (mustache_cache_entry*)(cache->entryBuffer.u + block->offset);
    mustache_cache_entry* entryEnd = (mustache_cache_entry*)(entry + block->entryCount);

    for (; entry < entryEnd; entry++)
    {
        if (entry->key.len != itemKey.len) {
            continue;
        }
        if (strneql(entry->key.u, itemKey.u, itemKey.len))
        {
            // remove item
            // shift everything about the entry down
            memmove(entry - 1, entry, (entryEnd - entry - 1) * sizeof(*entry));
            block->entryCount--;
            if (block->entryCount == 0) {
                block->capacity = 0;
                block->offset = 0;
                return MUSTACHE_SUCCESS;
            }
            block->capacity = u32_round_to_next_power_of_2(block->entryCount * sizeof(mustache_cache_entry));
            return MUSTACHE_SUCCESS;
        }
    }

    return MUSTACHE_SUCCESS;
}



static int fseek_callback(void* udata, int64_t whence, MUSTACHE_SEEK_DIR seekdir)
{
    FILE* fptr = (FILE*)udata;
    return _fseeki64(fptr, whence, seekdir);
}

static size_t fread_callback(void* udata, uint8_t* dst, size_t dstlen)
{
    FILE* fptr = (FILE*)udata;
    return fread(dst, 1, dstlen, fptr);
}



uint8_t mustache_parse_file(mustache_parser* parser, mustache_const_slice filename, mustache_template_cache* streamCache, MUSTACHE_CACHE_MODE cacheMode, mustache_param* params, mustache_slice sourceBuffer, mustache_slice parseBuffer, void* uData, mustache_parse_callback parseCallback)
{   
   
#ifndef NDEBUG
    if (filename.len > 2048) {
        assert(00 && "mustache_parse_file: SUCH A LARGE FILENAME MAY RESULT IN PROGRAM INSTABILITY!");
    }
#endif
    // tragically fopen requires a null terminated string, and there is no workaround even for an all-around 
    // system specific design, it's possible on Windows but not on Linux to my knowledge :(
    uint8_t* filenameNT = _alloca(filename.len+1);
    memcpy(filenameNT, filename.u, filename.len);
    filenameNT[filename.len] = 0;

    FILE* fptr;
    fptr = fopen(filenameNT, "rb");
    if (!fptr) {
        return MUSTACHE_ERR_FILE_OPEN;
    }

    // parse in chunks
    mustache_stream stream = {
        .udata = fptr,
        .readCallback = fread_callback,
        .seekCallback = fseek_callback,
    };

    return mustache_parse_stream(parser, &stream, streamCache, cacheMode, params, parseBuffer, sourceBuffer, uData, parseCallback);
}


static uint8_t* get_truthy_close(mustache_const_slice paramName, uint8_t* cur, uint8_t* end)
{
#ifndef NDEBUG
    if (cur == NULL || end < cur || end == NULL)
    {
        assert(00 && "get_truthy_close: INVALID CUR OR END ARGUMENT");
    }
#endif // !NDEBUG

    while (cur < end-2)
    {
        if (*cur == '{' && *(cur + 1) == '{' && *(cur + 2) == '/')
        {
            // find end
            cur += 3;

            uint8_t* cur2 = cur;
            while (cur2<end-1)
            {
                if (*cur2 == '}' && *(cur2 + 1) == '}')
                {
                    break;
                }
                cur2++;
            }

          
            uint16_t dist = cur2 - cur;
            if (paramName.len == dist && strneql(paramName.u, cur, dist))
            {
                return cur-3;
            }
        }
        cur++;
    }
    return NULL;
}

uint8_t stream_cache_add_var(mustache_template_cache* streamCache, mustache_slice srcBuffer, uint8_t* varBegin, uint8_t* varLast, uint8_t* truthyClose)
{
    uint32_t offsetBegin = varBegin - srcBuffer.u;
    uint32_t offsetEnd = varLast - srcBuffer.u;

    if (streamCache->varCount==streamCache->privMAX_VAR_COUNT) {
        return MUSTACHE_ERR_NO_SPACE;
    }
    else {
        mustache_var_info* vars = (mustache_var_info*)(streamCache->varBuffer.u);
        uint32_t truthyLen;
        if (truthyClose) {
            truthyLen = truthyClose - (varLast);
        }
        else {
            truthyLen = 0;
        }
        vars[streamCache->varCount] = (mustache_var_info){ offsetBegin, offsetEnd, truthyLen};
        streamCache->varCount++;
        return MUSTACHE_SUCCESS;
    }
};

static uint8_t* get_line_end(uint8_t* line, uint8_t* searchEnd)
{
    while (line<searchEnd)
    {
        if (*line == '\n') {
            return line;
        }
        line++;
    }
    return NULL;
}

static uint8_t* get_line_begin(uint8_t* line, uint8_t* searchBegin) {
    while (line!=searchBegin-1)
    {
        if (*line == '\n') {
            line++;
            break;
        }
        line--;
    }
    return line;
}

static MUSTACHE_TYPE get_mustache_type(uint8_t* mustacheOpen) {
    switch (*mustacheOpen)
    {
    case '!':
        return MUSTACHE_TYPE_COMMENT;
    case '^':
        return MUSTACHE_TYPE_FALSY;
    case '#':
        return MUSTACHE_TYPE_POUND;
    case '/':
        return MUSTACHE_TYPE_CLOSE;
    default:
        return MUSTACHE_TYPE_VARIABLE;
    }
}
static bool is_line_standalone(uint8_t* line, uint8_t* lineEnd, uint8_t* mustacheIntBegin, uint8_t* mustacheIntLast)
{
#ifndef NDEBUG
    if (*mustacheIntBegin == '{' || *mustacheIntLast == '}') {
        assert(00 && "is_line_standalone: mustacheIntBegin AND mustacheIntLast MUST BE POINTERS TO THE INTERIOR OF A MUSTACHE");
    }
#endif // !NDEBUG

    mustacheIntBegin -= 2;
    while (line < mustacheIntBegin)
    {
        if (!isspace(*line)) {
            return false;
        }
        line++;
    }

    mustacheIntLast += 3;
    line = mustacheIntLast;
    while (line < lineEnd)
    {
        if (!isspace(*line)) {
            return false;
        }
        line++;
    }

    return true;
}

static bool is_mustache_open(uint8_t* s) {
#ifndef NDEBUG
    if (s == NULL) {
        assert(00 && "is_mustache_open: s MUST NOT BE A VALID POINTER.");
    }
#endif // !NDEBUG
    if (*s == '{' && *(s + 1) == '{') {
        return true;
    }
    return false;
}

static bool is_mustache_close(uint8_t* s) {
#ifndef NDEBUG
    if (s == NULL) {
        assert(00 && "is_mustache_open: s MUST NOT BE A VALID POINTER.");
    }
#endif // !NDEBUGs
    if (*s == '}' && *(s + 1) == '}') {
        return true;
    }
    return false;
}

static mustache_param* get_parameter(uint8_t* nameBegin, uint8_t nameLen, mustache_param* globalParams, parent_stack* parentStack)
{
    // TRAVERSE PARENT STACK
    for (int32_t i = parentStack->count-1; i >= 0; i--) {
        mustache_param* parentNode = ((mustache_param**)parentStack->buf.u)[i];
        mustache_param* node;
        uint32_t MAX_COUNT;

        if (parentNode->type == MUSTACHE_PARAM_LIST) {
            mustache_param_list* list = (mustache_param_list*)parentNode;
            MAX_COUNT = list->valueCount;
            node = list->pValues;
        }
        else if (parentNode->type == MUSTACHE_PARAM_OBJECT) {
            mustache_param_object* obj = (mustache_param_object*)obj;
            node = obj->pMembers;
            MAX_COUNT = UINT32_MAX;
        }
        else {
#ifndef NDEBUG
            assert(00 && "get_parameter: PARENT STACK IS CORRUPTED.");
#endif // !NDEBUG
            return NULL;
        }




        uint32_t c = 0;
        while (node&&c<MAX_COUNT)
        {
            if (node->name.len == nameLen &&
                strneql(node->name.u, nameBegin, nameLen))
            {
                return node;
            }
            node = node->pNext;
            c++;
        }
    }

    // TRAVERSE GLOBAL PARAMS
    while (globalParams) {
        if (globalParams->name.len == nameLen &&
            strneql(globalParams->name.u, nameBegin, nameLen))
        {
            return globalParams;
        }

        globalParams = globalParams->pNext;
    }

    return NULL;
}

static mustache_param* get_child_param(uint8_t* nameBegin, uint8_t nameLen, mustache_param* parent)
{

    mustache_param* node;
    uint32_t MAX_COUNT;

    if (parent->type == MUSTACHE_PARAM_LIST) {
        mustache_param_list* list = (mustache_param_list*)parent;
        MAX_COUNT = list->valueCount;
        node = list->pValues;
    }
    else if (parent->type == MUSTACHE_PARAM_OBJECT) {
        mustache_param_object* obj = (mustache_param_object*)obj;
        node = obj->pMembers;
        MAX_COUNT = UINT32_MAX;
    }
    else {
#ifndef NDEBUG
        assert(00 && "get_child_param: parent IS NOT A PARENT.");
#endif // !NDEBUG
    }

    uint32_t ni = 0;
    while (node && ni < MAX_COUNT)
    {
        if (node->name.len == nameLen && strneql(node->name.u, nameBegin, nameLen)) {
            return node;
        }
        node = node->pNext;
        ni++;
    }
    return NULL;
}

static uint8_t* write_variable(mustache_param* paramBASE, uint8_t* outputHead, uint8_t* outputEnd)
{
    if (paramBASE->type == MUSTACHE_PARAM_NUMBER) {
        mustache_param_number* param = (mustache_param_number*)paramBASE;
        outputHead = dtoa(param->value, outputHead, (size_t)(outputEnd - outputHead),
            param->decimals, param->trimZeros);
    }
    else if (paramBASE->type == MUSTACHE_PARAM_BOOLEAN) {
        mustache_param_boolean* param = (mustache_param_boolean*)paramBASE;
        if (param->value) {
            uint8_t dist = min((int8_t)strlen("true"), outputEnd - outputHead);
            memcpy(outputHead, "true", dist);
            outputHead += dist;
        }
        else {
            uint8_t dist = min((int8_t)strlen("false"), outputEnd - outputHead);
            memcpy(outputHead, "false", dist);
            outputHead += dist;
        }
    }
    else if (paramBASE->type == MUSTACHE_PARAM_STRING) {
        mustache_param_string* param = (mustache_param_string*)paramBASE;

        uint32_t distToEnd = outputEnd - outputHead;
        uint32_t dist = min(distToEnd, param->str.len);
        memcpy(outputHead, param->str.u, dist);
        outputHead += dist;
    }
   
    return outputHead;
}

static uint8_t evaluate_parameter(mustache_slice sourceBuffer, uint8_t* sourceEnd, mustache_template_cache* cache, MUSTACHE_CACHE_MODE cacheMode, mustache_param* paramBase, 
    uint8_t** outputHead, uint8_t* outputEnd, uint8_t* mustacheOpen, uint8_t** mustacheClose, uint8_t type, uint8_t** truthyClose, uint32_t arrIndex, parent_stack* parentStack)
{
    uint16_t mustacheLen = *mustacheClose - mustacheOpen + 1;
    paramBase = get_parameter(mustacheOpen, mustacheLen, paramBase, parentStack);
    if (!paramBase) {
        return MUSTACHE_ERR_NONEXISTENT;
    }

  
    // LIST PARAM
    if (paramBase->type == MUSTACHE_PARAM_LIST)
    {
        mustache_param_list* param = (mustache_param_list*)paramBase;
        mustache_param* curParam=param->pValues;

        uint8_t* searchPtr = *mustacheClose;
        // technically this is a loop close, not truthy/conditional
        *truthyClose = get_truthy_close(paramBase->name, *mustacheClose, sourceEnd);
        uint8_t* mOpen = searchPtr;
        uint8_t* mClose = NULL;
        while (searchPtr < *truthyClose - 2) {
            if (is_mustache_open(searchPtr)) {
                mOpen = searchPtr;
                break;
            }
            searchPtr++;
        }
        if (mOpen == *mustacheClose) {
            return MUSTACHE_ERR_INVALID_TEMPLATE;
        }

        while (searchPtr < *truthyClose - 2)
        {
            if (is_mustache_close(searchPtr)) {
                mClose = searchPtr;
                break;
            }
            searchPtr++;
        }
        if (mClose == NULL) {
            return MUSTACHE_ERR_INVALID_TEMPLATE;
        }

        if (parent_stack_push(parentStack, param)) {
            return MUSTACHE_ERR_OVERFLOW;
        }

        uint8_t* truthyInteriorBegin = mustacheOpen + 2 + param->name.len;
        uint8_t* truthyInteriorLast = *truthyClose - 1;



        mOpen += 2;
        mClose -= 1;
        uint16_t paramLen = mClose - mOpen + 1;
        uint32_t i = 0;
        while (curParam&& i < param->valueCount && searchPtr < sourceEnd)
        {
            if (paramLen == 1 && mOpen[0]=='.') {
                // write the txt before the {{.}}
                uint32_t dist = (mOpen-2) - truthyInteriorBegin;
                memcpy(*outputHead, truthyInteriorBegin, dist);
                *outputHead += dist;

                // write var
                *outputHead = write_variable(curParam, *outputHead, outputEnd);
                // write the txt after the {{.}}
                dist = truthyInteriorLast - (mClose+3);
                memcpy(*outputHead, mClose + 3, dist);
                *outputHead += dist;
            }
            else {
                // EVALUATE PARAM
            }

            curParam = curParam->pNext;
            ++i;
        }

        parent_stack_pop(parentStack, param);

        // MOVE INPUT HEAD
        *truthyClose = *truthyClose + 2 + param->name.len;
        *mustacheClose = *truthyClose;


    }
    // STRING PARAM
    else if (paramBase->type == MUSTACHE_PARAM_STRING)
    {
        *outputHead = write_variable(paramBase, *outputHead, outputEnd);
        if (cache && cacheMode & MUSTACHE_CACHE_MODE_WRITE) {
            stream_cache_add_var(cache, sourceBuffer, mustacheOpen, *mustacheClose, NULL);
        }
    }
    // NUMBER PARAM
    else if (paramBase->type == MUSTACHE_PARAM_NUMBER)
    {
        *outputHead = write_variable(paramBase, *outputHead, outputEnd);
        if (cache && cacheMode & MUSTACHE_CACHE_MODE_WRITE) {
            stream_cache_add_var(cache, sourceBuffer, mustacheOpen, *mustacheClose, NULL);
        }
    }
    // BOOLEAN PARAM
    else if (paramBase->type == MUSTACHE_PARAM_BOOLEAN) {
        mustache_param_boolean* param = (mustache_param_boolean*)paramBase;

        if (type != MUSTACHE_TYPE_VARIABLE)
        {
            *truthyClose = get_truthy_close(paramBase->name, *mustacheClose, sourceEnd);
            if (!truthyClose) {
                return MUSTACHE_ERR_INVALID_TEMPLATE;
            }
            uint8_t* interiorFirst = *mustacheClose + 3;
            uint8_t* interiorLast = *truthyClose;



            bool standlone = false;
            uint8_t* lineEnd;
            // check if the starting cond. line is standalone. If so, skip it.
            uint8_t* lineBeg = get_line_begin(mustacheOpen, sourceBuffer.u);
            lineEnd = get_line_end(mustacheOpen, sourceEnd);
            if (lineEnd) {
                standlone = is_line_standalone(lineBeg, lineEnd, mustacheOpen - 1, *mustacheClose);
                if (standlone) {
                    *mustacheClose = lineEnd;
                }
            }

            // check if the ending cond. line is standalone. If so, skip it.
            lineBeg = get_line_begin(*truthyClose, sourceBuffer.u);
            lineEnd = get_line_end(*truthyClose, sourceEnd);
            if (lineEnd) {
                standlone = is_line_standalone(lineBeg, lineEnd,*truthyClose+2, *truthyClose+2+param->name.len);
                if (standlone) {
                    *mustacheClose = lineEnd-3;
                }
            }
            if ((type == MUSTACHE_TYPE_POUND && param->value) || (type == MUSTACHE_TYPE_FALSY && !param->value)) {
                // IF CONDITION IS TRUE
                *truthyClose = *truthyClose + 2 + param->name.len;
            }
            else {
                // IF CONDITION IS FALSE
                *truthyClose = *truthyClose + 2 + param->name.len;
                *mustacheClose = *truthyClose;
                goto skip;
            }

        }
        else {
            *outputHead = write_variable(paramBase, *outputHead, outputEnd);
            if (cache && cacheMode & MUSTACHE_CACHE_MODE_WRITE) {
                stream_cache_add_var(cache, sourceBuffer, mustacheOpen, *mustacheClose, NULL);
            }
        }
    }
  
skip:
    return MUSTACHE_SUCCESS;
}

/*
uint8_t mustache_parse_from_cache(mustache_parser* parser, mustache_stream* stream, mustache_template_cache* cache,
    mustache_param* params, mustache_slice sourceBuffer, mustache_slice outputBuffer, void* uData, mustache_parse_callback parseCallback)
{
#ifndef NDEBUG
    memset(sourceBuffer.u, 0, sourceBuffer.len);
    memset(outputBuffer.u, 0, outputBuffer.len);
#endif // !NDEBUG

   
    size_t readBytes = stream->readCallback(stream->udata, sourceBuffer.u, sourceBuffer.len);

    if (readBytes < 4 || readBytes >= UINT32_MAX - 3) {
        return MUSTACHE_ERR_ARGS;
    }

    uint8_t* outputHead = outputBuffer.u;
    uint8_t* inputHead = sourceBuffer.u;
    uint8_t* sourceEnd = sourceBuffer.u + sourceBuffer.len;
    uint8_t* outputEnd = outputBuffer.u + outputBuffer.len;

    for (uint32_t i = 0; i < cache->varCount; ++i)
    {
        mustache_var_info* varInfo = (mustache_var_info*)cache->varBuffer.u + i;
        if (varInfo->begOffset < 2) {
            return MUSTACHE_ERR_INVALID_CACHE;
        }

        uint32_t A = (inputHead - sourceBuffer.u);
        if (varInfo->begOffset<2 || A>(varInfo->begOffset)) {
            continue;
        }
        uint32_t dist = (varInfo->begOffset) - A - 2;

        memcpy(outputHead, inputHead, dist);
        inputHead = sourceBuffer.u + varInfo->begOffset;
        outputHead += dist;

        uint8_t* mustacheOpen = inputHead;
        uint8_t* mustacheClose = sourceBuffer.u + varInfo->lastOffset;
        uint16_t mustacheLen = (mustacheClose - mustacheOpen) + 1;

        if (*mustacheOpen == '!') { // skip comments
            uint8_t* le = get_line_end(inputHead, sourceEnd);
            if (is_line_standalone(get_line_begin(inputHead,sourceBuffer.u),le)) {
                inputHead = le + 1;
            }
            inputHead += mustacheLen + 2;
            continue;
        }

        uint8_t truthyState = 0; // 0: none
                                 // 1: truthy
                                 // 2: inverted truthy
        if (*mustacheOpen == '#') {
            mustacheOpen++;
            mustacheLen--;
            truthyState = 1;
        }
        else if (*mustacheOpen == '^') {
            mustacheOpen++;
            mustacheLen--;
            truthyState = 2;
        }

        uint8_t* truthyClose;
        evaluate_parameter(
            sourceBuffer, sourceEnd, NULL,
            MUSTACHE_CACHE_MODE_NONE, params, &outputHead,
            outputEnd, mustacheOpen, &mustacheClose,
            mustacheLen, truthyState, &truthyClose,
            0, NULL
        );

        if (truthyState != 0) {
            inputHead = truthyClose+3;
            int i=0;
        }
        else {
            inputHead = mustacheClose + 3;
        }
    }

    uint32_t a = inputHead - sourceBuffer.u;
    uint32_t dist = (sourceBuffer.u + readBytes - inputHead);
    memcpy(outputHead, inputHead, dist);
    outputHead += dist;


    mustache_slice parsedSlice = {
        .u = outputBuffer.u,
        .len = outputHead - outputBuffer.u,
    };
    parseCallback(parser, uData, parsedSlice);

    return MUSTACHE_SUCCESS;
}
*/

uint8_t mustache_parse_stream(mustache_parser* parser, mustache_stream* stream, mustache_template_cache* streamCache, MUSTACHE_CACHE_MODE cacheMode,
    mustache_param* params, mustache_slice sourceBuffer, mustache_slice outputBuffer, void* uData, mustache_parse_callback parseCallback)
{
    if (streamCache) {
        streamCache->privMAX_VAR_COUNT = (uint32_t)(streamCache->varBuffer.len / sizeof(mustache_var_info));
        if (cacheMode & MUSTACHE_CACHE_MODE_READ) {
            uint8_t err=0;
            //uint8_t err = mustache_parse_from_cache(parser, stream, streamCache, params, sourceBuffer, outputBuffer, uData, parseCallback);
            if (err != MUSTACHE_ERR_INCOMPLETE) {
                return err;
            }
        }
    }

    uint8_t* inputHead = sourceBuffer.u;
    uint8_t* inputBeg = sourceBuffer.u;
    uint8_t* inputEnd = sourceBuffer.u + sourceBuffer.len;
    uint8_t transientBuffer[2048];
    uint16_t transientBufferLen = 0;

    uint8_t* mustacheOpen = NULL;
    uint8_t* mustacheClose = sourceBuffer.u;

    size_t readBytes = stream->readCallback(stream->udata, sourceBuffer.u, sourceBuffer.len);

    if (readBytes < 4 || readBytes >= UINT32_MAX-3) {
        return MUSTACHE_ERR_ARGS;
    }

    uint8_t* outputHead = outputBuffer.u;
    uint8_t* outputEnd = outputBuffer.u + outputBuffer.len;

    parent_stack parStack = {
        .buf = parser->parentStackBuf,
        .count = 0,
        .MAX_COUNT = parser->parentStackBuf.len / sizeof(void*)
    };

    
    uint8_t* lastInputHead = inputHead;
    while (inputHead < inputEnd)
    {
        if (is_mustache_open(inputHead))
        {
            if (mustacheClose <= inputHead) {
                size_t s = inputHead - mustacheClose;
                memcpy(outputHead, mustacheClose, s);
                outputHead += s;
                mustacheOpen = inputHead + 2;
            }

            

            // get mustache close
            while (inputHead < inputEnd)
            {
                if (is_mustache_close(inputHead))
                {
                    mustacheClose = inputHead - 1;

                    MUSTACHE_TYPE type = get_mustache_type(mustacheOpen);
                    if (type == MUSTACHE_TYPE_COMMENT || type == MUSTACHE_TYPE_CLOSE) {
                    }
                    else if (type == MUSTACHE_TYPE_VARIABLE) {
                        uint8_t* tc;
                        // evaluate
                        evaluate_parameter(
                            sourceBuffer, inputEnd, streamCache,
                            cacheMode, params, &outputHead,
                            outputEnd, mustacheOpen, &mustacheClose,
                            type, &tc, 0, &parStack
                        );
                    }
                    else 
                    {
                        mustacheOpen++; // skip prefix ('!', '^', '#', '/')

                        uint8_t* tc=NULL;
                        // evaluate
                        evaluate_parameter(
                            sourceBuffer, params, &inputHead, &outputHead,
                            outputEnd, mustacheOpen, &mustacheClose,
                            type, &tc, 0, &parStack
                        );
                    }

                    inputHead = mustacheClose;
                    break;
                }
                inputHead++;
            }
        }

        inputHead++;
    }



    /*
    while (true)
    {
        // look for {{}};
        for (uint32_t i = 0; i < sourceBuffer.len+1; ++i)
        {
            if (sourceBuffer.u[i] == '{' &&
                sourceBuffer.u[i+1] == '{')
            {
                size_t s;
                if (mustacheClose > sourceBuffer.u + i) {
                    s = 0;
                } else {
                    s = sourceBuffer.u + i - mustacheClose;
                }
                memcpy(outputHead, mustacheClose, s);
                mustacheOpen = sourceBuffer.u + i + 2;
                outputHead += s; 

                for (uint32_t j = i + 2; j < sourceBuffer.len; ++j) {
                    if (sourceBuffer.u[j - 1] == '}' &&
                        sourceBuffer.u[j] == '}')
                    {
                        mustacheClose = sourceBuffer.u + j - 2;

                        if (*mustacheOpen == '!') { // skip comments
                            uint8_t* le = get_line_last(inputHead, sourceEnd);
                            if (is_line_standalone(get_line_begin(inputHead, sourceBuffer.u), le)) {
                                inputHead = le + 1;
                            }

                            if (streamCache && cacheMode & MUSTACHE_CACHE_MODE_WRITE) {
                                stream_cache_add_var(streamCache, sourceBuffer, mustacheOpen, mustacheClose, NULL);
                            }
                            break;
                        }
                      
                        uint8_t truthyState = 0; // 0: none
                                                 // 1: truthy
                                                 // 2: inverted truthy
                        if (*mustacheOpen == '#') {
                            mustacheOpen++;
                            truthyState = 1;
                        }
                        else if (*mustacheOpen == '^') {
                            mustacheOpen++;
                            truthyState = 2;
                        }
                        uint16_t mustacheLen = (mustacheClose - mustacheOpen) + 1;

                        uint8_t* tc;
                        evaluate_parameter(
                            sourceBuffer, sourceEnd, streamCache,
                            cacheMode, params, &outputHead,
                            outputEnd, mustacheOpen, &mustacheClose,
                            mustacheLen, truthyState, &tc,
                            0, &parStack
                        );
                        mustacheClose += 3;
                        j = (uint32_t)(mustacheClose - sourceBuffer.u);


                        mustacheOpen = NULL;
                        i = j;
                        break;
                    }
                }
            }
        }
        if (mustacheOpen) {
            transientBufferLen = (uint16_t)((sourceBuffer.u + readBytes) - mustacheOpen);
            if (transientBufferLen > sizeof(transientBuffer)) {
                transientBufferLen = sizeof(transientBuffer);
            }
            memcpy(transientBuffer, mustacheOpen, transientBufferLen);
        }
        */
        // fill the last part of the output buffer if needed
        size_t s = sourceBuffer.u + readBytes - mustacheClose;
        memcpy(outputHead, mustacheClose, s);
        outputHead += s;

        mustache_slice parsedSlice = {
            .u = outputBuffer.u,
            .len = outputHead - outputBuffer.u,
        };
        parseCallback(parser, uData, parsedSlice);

        readBytes = stream->readCallback(stream->udata, sourceBuffer.u, sourceBuffer.len);
        if (readBytes == 0) {
            //break;
        }
    //}

    return MUSTACHE_SUCCESS;
}





/* ====== SYSTEM TESTS ====== */

#ifdef MUSTACHE_SYSTEM_TESTS
void mustache_cache_print_first_byte_lookup(const mustache_cache* cache)
{
    printf("\nCACHE can hold a maximum of %d lookup entries.\n", cache->entryBuffer.len / sizeof(mustache_cache_entry));
    for (uint32_t i = 0; i < 256; ++i) {
        const mustache_cache_lookup_block* block = cache->firstByteLookup + i;
        if (block->capacity > 0) {
            printf("==== FIRST BYTE LOOKUP BLOCK #%d ====\n", i);
            printf("\tCAPACITY: %d\n", block->capacity);
            printf("\tOFFSET: %d\n", block->offset);
            printf("\tENTRIES: %d\n", block->entryCount);
        }
    }
}


void mustache_cache_print_entries_of_lookup_block(const mustache_cache* cache, uint8_t byte)
{
    uint32_t i = 0;
    mustache_cache_entry* entry = (mustache_cache_entry*)(cache->entryBuffer.u + cache->firstByteLookup[byte].offset);
    printf("==== BLOCK '%c'(#%d) ENTRY COUNT: %d ====\n", byte, byte, cache->firstByteLookup[byte].entryCount);
    for (uint8_t i = 0; i < cache->firstByteLookup[byte].entryCount; ++i) 
    {
        printf("\t==== CACHE ENTRY #%d ====\n",i);
        printf("\t\tNAME: %.*s\n", entry->key.len,entry->key.u);
        printf("\t\tDATA LEN.: %d\n", entry->item.dataLen);

        entry++;
    }
}

void mustache_cache_validate(const mustache_cache* cache)
{
    uint16_t invalidLookupBlockCount = 0;
    uint16_t blocksWithContentsBlockCount = 0;
    bool isInvalid = false;
    for (uint16_t b = 0; b < array_count(cache->firstByteLookup); ++b)
    {
        isInvalid = false;
        const mustache_cache_lookup_block* block = cache->firstByteLookup + b;
        if (block->capacity) 
        {
            blocksWithContentsBlockCount++;
            if (block->offset + block->capacity > cache->entryBuffer.len) 
            {
                printf("CACHE VALIDATION: BLOCK '%c'(#%d) OVERFLOWS CACHE ENTRY BUFFER:\n", b, b);
                printf("\tENTRY BUFFER SIZE: %d - BLOCK EXTENDS THIS BY %d BYTES\n", cache->entryBuffer.len, (block->offset + block->capacity) - cache->entryBuffer.len);
                isInvalid = true;
            }

            // CHECK OVERLAP
            const mustache_cache_lookup_block* blockB = cache->firstByteLookup;
            const mustache_cache_lookup_block* blockB_end = cache->firstByteLookup + array_count(cache->firstByteLookup);
            for (; block < blockB_end; ++block) 
            {
                if (blockB != block && blockB->capacity > 0) 
                {
                    if (blockB->offset >= block->offset && blockB->offset <= block->offset + block->capacity
                        ||
                        blockB->offset + blockB->capacity >= block->offset && blockB->offset + blockB->capacity <= block->offset + block->capacity)
                    {
                        const uint8_t bb = blockB - cache->firstByteLookup;
                        printf("CACHE VALIDATION: BLOCK '%c'(#%d) COLLIDES WITH BLOCK '%c'(#%d):\n", b, b, bb, bb);
                        printf("\tBLOCK BLOCK '%c'(#%d): OFFSET OF %d, CAPACITY OF %d", b, b, block->offset, block->capacity);
                        printf("\tBLOCK BLOCK '%c'(#%d): OFFSET OF %d, CAPACITY OF %d", bb, bb, block->offset, block->capacity);
                        isInvalid = true;
                    }
                }
            }

            if (isInvalid) {
                invalidLookupBlockCount++;
            }
        }
    }

    if (invalidLookupBlockCount) {
        printf("CACHE VALIDATION COMPLETE: %d OF %d LOOKUP BLOCKS ARE INVALID\n", invalidLookupBlockCount, blocksWithContentsBlockCount);
    }
    else {
        printf("CACHE VALIDATION COMPLETE: ALL %d BLOCKS ARE VALID\n", blocksWithContentsBlockCount);
    }
}

void mustache_cache_print_entries(const mustache_cache* cache)
{
    const mustache_cache_lookup_block* block = cache->firstByteLookup;
    for (; block<cache->firstByteLookup+array_count(cache->firstByteLookup); ++block)
    {
        uint8_t b = block - cache->firstByteLookup;
        if (block->capacity > 0) {
            mustache_cache_print_entries_of_lookup_block(cache, b);
        }
    }
}

#endif