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

typedef enum
{
    STRUCTURE_TYPE_VAR,
    STRUCTURE_TYPE_SCOPED_POUND,
    STRUCTURE_TYPE_SCOPED_CARET,
    STRUCTURE_TYPE_COMMENT,
    STRUCTURE_TYPE_CLOSE,
    STRUCTURE_TYPE_SKIP_RANGE,
    STRUCTURE_TYPE_ROOT
} STRUCTURE_TYPE;

typedef struct {
    uint32_t lineBegin;
    uint32_t lineEnd;
} standalone_data;

typedef struct structure structure;
typedef struct structure {
    structure* pNext;
    structure* pLast;
    STRUCTURE_TYPE type;

    uint32_t contentsFirst; // the first byte after the second '{'
    uint32_t contentsEnd; // the first closing '}'

    mustache_param* param;
    standalone_data* standalone;
} structure;

typedef struct {
    structure* pNext;
    structure* pLast;
    STRUCTURE_TYPE type;

    uint32_t skipFirst;
    uint32_t skipLast;
} skip_range_structure;

typedef struct {
    structure* pNext;
    structure* pLast;
    STRUCTURE_TYPE type;

    uint32_t contentsFirst; // the first byte after the second '{'
    uint32_t contentsEnd; // the first closing '}'

    mustache_param* param;
    standalone_data* standalone;
} var_structure;

typedef struct {
    structure* pNext;
    structure* pLast;
    STRUCTURE_TYPE type;
    uint32_t contentsFirst; // the first byte after the second '{'
    uint32_t contentsEnd; // the first closing '}'

    mustache_param* param;
    standalone_data* standalone;

    uint32_t interiorFirst; // the first byte of the interior
    uint32_t interiorEnd; // the end of the interior - the first opening bracket of it's closing mustache '{{/name}}'

    uint32_t curIdx; // current child index
    mustache_param* curChild;
} scoped_structure;

typedef struct {
    structure* pNext;
    structure* pLast;
    STRUCTURE_TYPE type;

    uint32_t contentsFirst; // the first byte after the second '{'
    uint32_t contentsEnd; // the first closing '}'

    mustache_param* param;
    standalone_data* standalone;

    scoped_structure* parent;
} close_structure;


static void parent_stack_pop(parent_stack* stack)
{
#ifndef NDEBUG
    if (stack->count == 0) {
        assert(00 && "parent_stack_pop: INVALID CALL, STACK WILL UNDERFLOW!");
    }
#endif // !NDEBUG

    stack->count--;
}

static uint8_t parent_stack_push(parent_stack* stack, scoped_structure* parent)
{
#ifndef NDEBUG
    if (!(parent->type == STRUCTURE_TYPE_SCOPED_CARET || parent->type == STRUCTURE_TYPE_SCOPED_POUND)) {
        assert(00 && "parent_stack_push: param IS NOT A PARENT.");
    }
    mustache_param* p = parent->param;
    if (!(p->type == MUSTACHE_PARAM_LIST || p->type == MUSTACHE_PARAM_OBJECT)) {
        assert(00 && "parent_stack_push: param IS NOT A PARENT.");
    }
#endif // !NDEBUG

    if (stack->count == stack->MAX_COUNT) {
        return MUSTACHE_ERR_OVERFLOW;
    }

    uint8_t** pointers = (uint8_t**)stack->buf.u;
    pointers[stack->count] = (uint8_t*)parent;
    stack->count++;

    return MUSTACHE_SUCCESS;
};

//returns the last structure on the stack.
static scoped_structure* parent_stack_last(parent_stack* stack)
{
#ifndef NDEBUG
    if (stack->count == 0) {
        assert(00 && "parent_stack_last: INVALID CALL, stack.count MUST NOT BE EMPTY!");
    }
#endif // !NDEBUG

    scoped_structure** pointers = (scoped_structure**)stack->buf.u;
    return pointers[stack->count - 1];
}



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

static scoped_structure* get_scoped_close_parent(close_structure* close)
{
    structure* cur = (structure*)close;
    cur = cur->pLast;
    while (cur)
    {
        if (cur->param == close->param) {
            return (scoped_structure*)cur;
        }
        cur = cur->pLast;
    }
    return NULL;
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
    return line;
}

static uint8_t* get_line_begin(uint8_t* line, uint8_t* searchBegin) {
    while (line!=searchBegin)
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
static bool is_line_standalone(uint8_t* line, uint8_t* lineEnd)
{
    bool isInMustache = false;
    while (line < lineEnd)
    {
        if (line < lineEnd - 2) {
            if (*line == '{' && *(line + 1) == '{')
            {
                uint8_t prefix = *(line + 2);
                if (!(prefix == '#' || prefix == '^' || prefix == '!' || prefix == '/'))
                {
                    return false;
                }
                isInMustache = true;
            }
            if (*(line + 1) == '}' && *(line + 2) == '}')
            {
                isInMustache = false;
                line += 3;
                if (line >= lineEnd) {
                    return true;
                }
            }
        }
        if (!isInMustache && !isspace(*line)) {
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

static mustache_param* get_parameter(const uint8_t* nameBegin, const uint8_t* nameEnd, mustache_param* globalParams, parent_stack* parentStack)
{
    // TRAVERSE PARENT STACK
    uint16_t nameLen = nameEnd - nameBegin;
    for (int32_t i = parentStack->count-1; i >= 0; i--) {
        scoped_structure* structNode = ((scoped_structure**)parentStack->buf.u)[i];
        mustache_param* parentNode = structNode->param;

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

static uint8_t is_parent(mustache_param* param) {
    if (param->type == MUSTACHE_PARAM_LIST || param->type == MUSTACHE_PARAM_OBJECT) {
        return true;
    }
    return false;
}


static uint8_t* get_mustache_close(uint8_t* inputHead, uint8_t* inputEnd) {
    while (inputHead < inputEnd)
    {
        if (is_mustache_close(inputHead)) {
            return inputHead;
        }
        inputHead++;
    }
    return NULL;
}

static uint8_t* get_scoped_interior_end(uint8_t* interiorFirst, uint8_t* end)
{
    uint32_t no = 1;
    while (interiorFirst <end)
    {
        uint8_t prefix = *(interiorFirst + 2);
        if (is_mustache_open(interiorFirst) && (prefix=='#' || prefix=='^')) {
            no++;
            interiorFirst += 2;
        }
        else if (is_mustache_open(interiorFirst) && prefix=='/') {
            no--;
            if (no == 0) {
                return interiorFirst;
            }
        }

        interiorFirst++;
    }
    return NULL;
}

static bool is_truthy(mustache_param* p)
{
    if (p->type == MUSTACHE_PARAM_STRING) {
        mustache_param_string* param = (mustache_param_string*)p;
        if (param->str.len > 0) {
            return true;
        } else {
            return false;
        }
    }
    else if (p->type == MUSTACHE_PARAM_BOOLEAN) {
        mustache_param_boolean* param = (mustache_param_boolean*)p;
        return param->value;
    }
    else if (p->type == MUSTACHE_PARAM_NUMBER) {
        mustache_param_number* param = (mustache_param_number*)p;
        return (bool)param->value;
    }
    else if (p->type == MUSTACHE_PARAM_LIST) {
        mustache_param_list* param = (mustache_param_list*)p;
        return param->valueCount;
    }
    if (p) {
        return true;
    }
    return false;
}

static uint8_t source_to_structured(mustache_parser* parser, structure* structureRoot, uint8_t* inputFirst, uint8_t* inputHead, uint8_t* inputEnd)
{
    structure* last_struct = structureRoot;
    while (inputHead<inputEnd)
    {
        if (is_mustache_open(inputHead))
        {
            uint8_t* first = inputHead+2;
            uint8_t* end = get_mustache_close(first, inputEnd);
            structure* mstruct = NULL;
            // handle escape case
            if (*(inputHead-1)=='/') {
                mstruct = parser->alloc(parser, sizeof(skip_range_structure));
                mstruct->type = STRUCTURE_TYPE_SKIP_RANGE;
                mstruct->pNext = NULL;

                skip_range_structure* asSkip = (skip_range_structure*)mstruct;
                asSkip->skipFirst = (inputHead - inputFirst)-1;
                asSkip->skipLast = inputHead - inputFirst;

                inputHead = end+2;
            }
            else if (*first == '/' || *first == '!')
            {
                if (*first == '/') {
                    mstruct = parser->alloc(parser, sizeof(close_structure));
                    if (!mstruct) {
                        return MUSTACHE_ERR_ALLOC;
                    }
                    mstruct->type = STRUCTURE_TYPE_CLOSE;
                    close_structure* asClosed = (close_structure*)mstruct;
                    asClosed->parent = NULL;
                }
                else {
                    mstruct = parser->alloc(parser, sizeof(structure));
                    if (!mstruct) {
                        return MUSTACHE_ERR_ALLOC;
                    }
                    mstruct->type = STRUCTURE_TYPE_COMMENT;
                }
                mstruct->param = NULL;
                mstruct->pNext = NULL;
           
                // check if it's standalone
                bool standlone = false;
                uint8_t* lineEnd;
                // check if the starting cond. line is standalone. If so, skip it.
                uint8_t* lineBeg = get_line_begin(first, inputFirst);
                lineEnd = get_line_end(first, inputEnd);
                if (lineEnd) {
                    standlone = is_line_standalone(lineBeg, lineEnd);
                    if (standlone) {
                      
                        mstruct->standalone = parser->alloc(parser,sizeof(standalone_data));
                        if (!mstruct->standalone) {
                            return MUSTACHE_ERR_ALLOC;}
                        mstruct->standalone->lineBegin = lineBeg - inputFirst;
                        mstruct->standalone->lineEnd = lineEnd - inputFirst;
                    }
                    else {
                        mstruct->standalone = NULL;
                    }
                }
                else {
                    mstruct->standalone = NULL;
                }
            }
            else if (*first == '^' || *first == '#')
            {
                mstruct = parser->alloc(parser, sizeof(scoped_structure));
                mstruct->param = NULL;
                mstruct->pNext = NULL;
                if (!mstruct) {
                    return MUSTACHE_ERR_ALLOC; }

                if (*first == '^') {
                    mstruct->type = STRUCTURE_TYPE_SCOPED_CARET;
                }
                else {
                    mstruct->type = STRUCTURE_TYPE_SCOPED_POUND;
                }

                // check if it's standalone
                bool standlone = false;
                uint8_t* lineEnd;
                // check if the starting cond. line is standalone. If so, skip it.
                uint8_t* lineBeg = get_line_begin(first, inputFirst);
                lineEnd = get_line_end(first, inputEnd);
                if (lineEnd) {
                    standlone = is_line_standalone(lineBeg, lineEnd);
                    if (standlone) {

                        mstruct->standalone = parser->alloc(parser, sizeof(standalone_data));
                        if (!mstruct->standalone) {
                            return MUSTACHE_ERR_ALLOC;
                        }
                        mstruct->standalone->lineBegin = lineBeg - inputFirst;
                        mstruct->standalone->lineEnd = lineEnd - inputFirst;
                    }
                    else {
                        mstruct->standalone = NULL;
                    }
                }
                else {
                    mstruct->standalone = NULL;
                }

                scoped_structure* asScoped = (scoped_structure*)mstruct;
                asScoped->interiorFirst = end - inputFirst + 2;
                uint8_t* int_end = get_scoped_interior_end(end+2,inputEnd);
                if (!int_end) {
                    asScoped->interiorEnd = 0;
                    return MUSTACHE_ERR_INVALID_TEMPLATE;
                }
                asScoped->interiorEnd = int_end - inputFirst;
            }
            else {
                mstruct = parser->alloc(parser, sizeof(var_structure));
                mstruct->param = NULL;
                mstruct->pNext = NULL;
                if (!mstruct) {
                    return MUSTACHE_ERR_ALLOC; }

                mstruct->type = STRUCTURE_TYPE_VAR;
            }

            if (mstruct->type != STRUCTURE_TYPE_SKIP_RANGE) {
                mstruct->contentsFirst = first - inputFirst;
                mstruct->contentsEnd = end - inputFirst;
            }
            inputHead = end + 1;

            last_struct->pNext = mstruct;
            mstruct->pLast = last_struct;
            last_struct = mstruct;
        }
        
        inputHead++;
    }
    
    return MUSTACHE_SUCCESS;
}


static uint8_t* mwrite(uint8_t* outputHead, uint8_t* outputEnd, const uint8_t* sourceBeg, const uint8_t* sourceEnd)
{
    if (sourceBeg > sourceEnd) {
        return outputHead;
    }
    
    size_t a = sourceEnd - sourceBeg;
    size_t b = outputEnd - outputHead;
    size_t s = min((sourceEnd- sourceBeg), (outputEnd- outputHead));
    memcpy(outputHead, sourceBeg, s);
    return outputHead+s;
}


void eval_jump(structure* mstruct, const uint8_t* m_name_first, const uint8_t* m_name_end, const uint8_t* input, uint8_t* outputEnd, uint8_t** outputHead, const uint8_t** lastNonEscaped) {
    if (mstruct->standalone) {
        const uint8_t* t = input + mstruct->standalone->lineBegin;
        if (t > *lastNonEscaped) {
            *outputHead = mwrite(*outputHead, outputEnd, *lastNonEscaped, t);
        }
        *lastNonEscaped = input + mstruct->standalone->lineEnd + 1;
    }
    else {
        const uint8_t* t = m_name_first - 3;
        if (t > *lastNonEscaped) {
            *outputHead = mwrite(*outputHead, outputEnd, *lastNonEscaped, t);
        }
        *lastNonEscaped = m_name_end + strlen("{{");
    }
}

static mustache_param* resolve_param_member(mustache_param* root, const uint8_t* strFirst, const uint8_t* strEnd)
{
#ifndef NDEBUG
    if (*strFirst !='.') {
        assert(00 && "resolve_param_member: strFirst must be a string parameter chain, i.e. '.member.name'");
    }
#endif // !NDEBUG

    if (strFirst+1 == strEnd) {
        return root;
    }

    const uint8_t* cur = strFirst;

    mustache_param* param = root;
    const uint8_t* lastDot = cur;
    cur++;
    while (cur <= strEnd)
    {
        if (*cur == '.' || cur == strEnd)
        {    
            const uint8_t* nameBegin = lastDot + 1;
            const uint8_t* nameEnd = cur;
            uint32_t nameLen = nameEnd - nameBegin;

            uint32_t i;
            if (param->type == MUSTACHE_PARAM_LIST) {
                mustache_param_list* asList = (mustache_param_list*)param;
                i = asList->valueCount;
            }
            else {
                i = UINT32_MAX;
            }

            param = ((mustache_param_list*)param)->pValues;
            while (param && i > 0)
            {
                if (nameLen == param->name.len && strneql(nameBegin, param->name.u, nameLen)) {
                    break;
                }
                param = param->pNext;
                i--;
            }

            if (!param) {
                return NULL;
            }
            lastDot = cur;
        }
        cur++;
    }

    return param;
}

uint8_t write_structured(mustache_slice outputBuffer, uint8_t** oh, mustache_const_slice inputBuffer, uint8_t* inputEnd, structure* structureRoot, 
                         mustache_param* globalParams, parent_stack* parentStack)
{
    structure* mstruct = structureRoot->pNext; // SKIP ROOT

    // INPUT
    const uint8_t* inputHead = inputBuffer.u;
    const uint8_t* input=inputBuffer.u;

    // OUTPUT
    uint8_t* outputHead = outputBuffer.u;
    uint8_t* outputEnd = outputBuffer.u + outputBuffer.len;

    const uint8_t* lastNonEscaped = inputHead;
    while (mstruct)
    {
        if (mstruct->type == STRUCTURE_TYPE_SKIP_RANGE) {
            skip_range_structure* asSkipRange = (skip_range_structure*)mstruct;
            const uint8_t* t = input + asSkipRange->skipFirst;
            inputHead = t;
            outputHead = mwrite(outputHead, outputEnd, lastNonEscaped, inputHead);
            t = input + asSkipRange->skipLast;
            lastNonEscaped = t;
        }
        else if (mstruct->type == STRUCTURE_TYPE_VAR) 
        {
            const uint8_t* m_name_first = input + mstruct->contentsFirst;
            const uint8_t* m_name_end = input + mstruct->contentsEnd;
            const uint32_t nameLen = m_name_end - m_name_first;

            outputHead = mwrite(outputHead, outputEnd, lastNonEscaped, m_name_first - 2);
            lastNonEscaped = m_name_end + strlen("{{");

            // HANDLE '.' CASE
            if (*m_name_first == '.')
            {
                scoped_structure* parent = parent_stack_last(parentStack);
                mustache_param* m_child = parent->curChild;
                // resolve '.' or chains '.member.name'
                mustache_param* member = resolve_param_member(m_child, m_name_first, m_name_end);
                if (member) {
                    outputHead = write_variable(member, outputHead, outputEnd);
                }
            }
            else if (!mstruct->param) {
                mstruct->param = get_parameter(m_name_first, m_name_end, globalParams, parentStack);
                if (!mstruct->param) {
                    goto skip_node;
                }
                outputHead = write_variable(mstruct->param, outputHead, outputEnd);
            }
        }
        else if (mstruct->type == STRUCTURE_TYPE_SCOPED_CARET || mstruct->type == STRUCTURE_TYPE_SCOPED_POUND) {
            scoped_structure* asScoped = (scoped_structure*)mstruct;
        
            const uint8_t* m_name_first = input + mstruct->contentsFirst+1;
            const uint8_t* m_name_end = input + mstruct->contentsEnd;

            if (!mstruct->param) {
                mstruct->param = get_parameter(m_name_first, m_name_end, globalParams, parentStack);
                if (!mstruct->param) {
                    goto skip_node;
                }
            }

            asScoped->curIdx = 0;
            
            if (mstruct->type == STRUCTURE_TYPE_SCOPED_POUND && is_parent(mstruct->param)) 
            {
                parent_stack_push(parentStack, asScoped);

                mustache_param_object* pAsObj = (mustache_param_object*)asScoped->param;
                asScoped->curChild = pAsObj->pMembers;

                if (mstruct->standalone) {
                    const uint8_t* t = input + mstruct->standalone->lineBegin;
                    outputHead = mwrite(outputHead, outputEnd, lastNonEscaped, t);
                    lastNonEscaped = input + mstruct->standalone->lineEnd + strlen("\n");
                }
                else {
                    const uint8_t* t = m_name_first - strlen("{{x");
                    outputHead = mwrite(outputHead, outputEnd, lastNonEscaped, t);
                    lastNonEscaped = m_name_end + strlen("{{");
                }
            } 
            else if (mstruct->type == STRUCTURE_TYPE_SCOPED_POUND || mstruct->type==STRUCTURE_TYPE_SCOPED_CARET) 
            {
                
                bool truthy = is_truthy(mstruct->param);
                if ((mstruct->type == STRUCTURE_TYPE_SCOPED_POUND && truthy) || (mstruct->type == STRUCTURE_TYPE_SCOPED_CARET && !truthy))
                {
                    if (mstruct->standalone) {
                        const uint8_t* t = input + mstruct->standalone->lineBegin;
                        outputHead = mwrite(outputHead, outputEnd, lastNonEscaped, t);
                        lastNonEscaped = input + mstruct->standalone->lineEnd + strlen("\n");
                    }
                    else {
                        const uint8_t* t = m_name_first - strlen("{{x");
                        outputHead = mwrite(outputHead, outputEnd, lastNonEscaped, t);
                        lastNonEscaped = m_name_end + strlen("{{");
                    }
                }
                else {
                    uint32_t nameLen = m_name_end - m_name_first;
                    const uint8_t* t;
                    if (mstruct->standalone) {
                        t = input + mstruct->standalone->lineBegin;
                    } else {
                        t = m_name_first - strlen("{{x");
                        //outputHead=write_identation(outputHead, );
                    }
                    outputHead = mwrite(outputHead, outputEnd, lastNonEscaped, t);
                    lastNonEscaped = input + asScoped->interiorEnd+nameLen+strlen("{{x}}");
                    int i = 0;
                }
            }
        }
        else if (mstruct->type == STRUCTURE_TYPE_COMMENT || mstruct->type == STRUCTURE_TYPE_CLOSE) 
        {
            const uint8_t* m_name_first = input + mstruct->contentsFirst+1;
            const uint8_t* m_name_end = input + mstruct->contentsEnd;

            if (mstruct->type == STRUCTURE_TYPE_CLOSE)
            {
                if (!mstruct->param) {
                    mstruct->param = get_parameter(m_name_first, m_name_end, globalParams, parentStack);
                }

                close_structure* asClose = (close_structure*)mstruct;
                if (!asClose->parent) {
                    asClose->parent = get_scoped_close_parent(asClose);
                }
                if (!asClose->parent) {
                    return MUSTACHE_ERR_INVALID_TEMPLATE;
                }

                scoped_structure* parent = asClose->parent;

                if (parent->param) {
                    if (parent->param->type == MUSTACHE_PARAM_LIST)
                    {
                        mustache_param_list* param = (mustache_param_list*)parent->param;
                        parent->curIdx++;
                        parent->curChild = parent->curChild->pNext;
                        if (parent->curIdx < param->valueCount && parent->curChild)
                        {

                            eval_jump(mstruct, m_name_first, m_name_end, input, outputEnd, &outputHead, &lastNonEscaped);
                            //outputHead = mwrite(outputHead, outputEnd, lastNonEscaped, m_name_first - strlen("{{/"));
                            // go to parent next again
                            mstruct = (structure*)parent;
                            eval_jump(mstruct, m_name_first, m_name_end, input, outputEnd, &outputHead, &lastNonEscaped);
                            goto skip_node;
                        }
                        else if (mstruct->param && is_parent(mstruct->param)) {
                            parent_stack_pop(parentStack);
                        }
                    }
                }


            }

            eval_jump(mstruct, m_name_first, m_name_end, input, outputEnd, &outputHead, &lastNonEscaped);
        }

    skip_node:
        mstruct = mstruct->pNext;
    }


    outputHead = mwrite(outputHead, outputEnd, lastNonEscaped, inputEnd);
    *oh = outputHead;
    return MUSTACHE_SUCCESS;
}

static void free_structure_chain(mustache_parser* p, structure* root)
{
    root = root->pNext;
    while (root)
    {

        structure* next = root->pNext;
        p->free(p,root);
        root = next;
    }
}

uint8_t mustache_parse_stream(mustache_parser* parser, mustache_stream* stream, mustache_template_cache* streamCache, MUSTACHE_CACHE_MODE cacheMode,
    mustache_param* params, mustache_slice inputBuffer, mustache_slice outputBuffer, void* uData, mustache_parse_callback parseCallback)
{
    uint8_t* inputHead = inputBuffer.u;
    uint8_t* inputBeg = inputBuffer.u;
    uint8_t transientBuffer[2048];
    uint16_t transientBufferLen = 0;

    uint8_t* mustacheOpen = NULL;
    uint8_t* mustacheClose = inputBuffer.u;

    size_t readBytes = stream->readCallback(stream->udata, inputBuffer.u, inputBuffer.len);
    uint8_t* inputEnd = inputBuffer.u + readBytes;

    if (readBytes < 4 || readBytes >= UINT32_MAX-3) {
        return MUSTACHE_ERR_ARGS;
    }

    uint8_t* outputHead = outputBuffer.u;
    uint8_t* outputEnd = outputBuffer.u + outputBuffer.len;

    parent_stack parentStack = {
        .buf = parser->parentStackBuf,
        .count = 0,
        .MAX_COUNT = parser->parentStackBuf.len / sizeof(void*)
    };

    structure structureRoot = { .type=STRUCTURE_TYPE_ROOT }; 
    MUSTACHE_RES err = source_to_structured(parser, &structureRoot, inputBuffer.u,inputHead,inputEnd);
    if (err) {
        return err;
    }

    err = write_structured(
        outputBuffer, &outputHead,
        (mustache_const_slice){ inputBuffer.u,inputBuffer.len },
        inputBuffer.u + readBytes,
        &structureRoot, params, &parentStack
    );

    if (err) {
        return err;
    }

    free_structure_chain(parser,&structureRoot);




    mustache_slice parsedSlice = {
        .u = outputBuffer.u,
        .len = outputHead - outputBuffer.u,
    };
    parseCallback(parser, uData, parsedSlice);

    readBytes = stream->readCallback(stream->udata, inputBuffer.u, inputBuffer.len);
    if (readBytes == 0) {
        //break;
    }
    //}

    return MUSTACHE_SUCCESS;
}