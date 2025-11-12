/*
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- 

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

-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*/

package not_mustache

import "core:odin/parser"
import "core:slice"

/* ====== ENUM TYPES ====== */

Err :: enum u8 {
    Success=0,
    Err,
    ErrAlloc,
    ErrFileOpen,
    ErrNonExistent,
    ErrNoSpace,
    ErrOverflow,
    ErrUnderflow,
    ErrIncomplete,
    ErrStream,
    ErrArgs,
    ErrTemplate,
    ErrJson
}

SeekDir :: enum {
    Set = 0,
    Cur = 1,
    End = 2,
    Len = 3
}

/* ====== FUNCTION CALLBACK TYPES ====== */

Alloc :: proc(parser: ^Parser, size: int) -> (rawptr);

Free :: proc(parser: ^Parser, size: int);

ParseCallback :: proc(parser: ^Parser, udata: rawptr, parsed: []u8);

SeekCallback :: proc(udata: rawptr, whence: i64, seekdir: SeekDir) -> (u64)

ReadCallback :: proc(udata: rawptr, dst: ^u8, dstlen: u64) -> (u64)


/* ====== STRUCTURE TYPES ====== */

Parser :: struct {
    userData: rawptr,
    parentStackBuffer: []u8,
    alloc: Alloc,
    free: Free
}

ParamType :: enum {
    None,
    Boolean,
    Number,
    String,
    List,
    Object
}

Param :: struct {
    pNext: ^Param,
    type: ParamType,
    name: string
}

ParamString :: struct {
    using param: Param,
    str: string
}

ParamNumber :: struct {
    using param: Param,
    value: f64,
    decimals: u8,
    trimZeros: bool
}

ParamBoolean :: struct {
    using param: Param,
    value: bool
}

ParamList :: struct {
    using param: Param,
    pValues: ^Param,
    valueCount: u32
}

ParamObject ::struct {
    using param: Param,
    pMembers: ^Param
};


Stream :: struct {
    udata: rawptr,
    readCallback: ReadCallback,
    seekCallback: SeekCallback
};

Structure :: struct {
    __A: rawptr,        // DO NOT ATTEMPT TO MODIFY THIS MEMBER, IT IS A PLACEHOLDER
    __B: rawptr,        // DO NOT ATTEMPT TO MODIFY THIS MEMBER, IT IS A PLACEHOLDER
    __C: Err,           // DO NOT ATTEMPT TO MODIFY THIS MEMBER, IT IS A PLACEHOLDER
    __D: u32,           // DO NOT ATTEMPT TO MODIFY THIS MEMBER, IT IS A PLACEHOLDER
    __E: u32,            // DO NOT ATTEMPT TO MODIFY THIS MEMBER, IT IS A PLACEHOLDER
    __F: u32,           // DO NOT ATTEMPT TO MODIFY THIS MEMBER, IT IS A PLACEHOLDER
    __G: rawptr,        // DO NOT ATTEMPT TO MODIFY THIS MEMBER, IT IS A PLACEHOLDER
    __H: rawptr,        // DO NOT ATTEMPT TO MODIFY THIS MEMBER, IT IS A PLACEHOLDER
}



foreign import not_mustache "not_mustache_bin:not_mustache.o"

foreign not_mustache {



/*****
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-

-+- Parses a mustache template source from disk. -+-

@param mustache_parser* parser
@param mustache_const_slice filename
@param mustache_structure* structChain - a pointer to a chain of mustache structures
@param mustache_param* params - the parameter chain
@param mustache_slice sourceBuffer - if the file length is larger than the source buffer, mustache_parse_file will return ERR_NO_SPACE
@param mustache_slice parseBuffer - where the parsed template will be stored
@param void* parseCallbackUdata - passed to the parseCallback function
@param mustache_parse_callback - called upon parse completion

@return uint8_t - MUSTACHE_RES return code.

-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*****/

@(link_name="mustache_parse_file")
parseFile :: proc (parser: ^Parser,  filename: []u8, structChain: ^Structure,  params: ^Param,
    sourceBuffer: []u8,  parseBuffer: []u8,  parseCallbackUdata: rawptr, parseCallback: ParseCallback) -> Err ---

/*****
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-

-+- Parses a mustache template source from an input stream. -+-

@param mustache_parser* parser
@param mustache_stream - the input stream to parse from
@param mustache_structure* structChain - a pointer to a chain of mustache structures
@param mustache_param* params - the parameter chain
@param mustache_slice sourceBuffer - if the stream length is larger than the source buffer, mustache_parse_file will return ERR_NO_SPACE
@param mustache_slice parseBuffer - where the parsed template will be stored
@param void* parseCallbackUdata - passed to the parseCallback function
@param mustache_parse_callback - called upon parse completion.

@return uint8_t - MUSTACHE_RES return code.      
      
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*****/

@(link_name="mustache_parse_stream")
parseStream :: proc (parser: ^Parser,  stream: ^Stream, structChain: ^Structure,  params: ^Param,
    sourceBuffer: []u8,  parseBuffer: []u8,  parseCallbackUdata: rawptr, parseCallback: ParseCallback) -> Err ---

/*****
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-

-+- Destroys a structure chain, calling parser->free for every node in the list. -+-

@param mustache_parser* parser
@param mustache_structure* structure_chain

-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*****/

@(link_name="mustache_structure_chain_free")
structureChainFree :: proc (parser: ^Parser, structChain: ^Structure) ---

/*****
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-

-+- Primes a structure chain for its next use, this must be called if the parameter -+-
    used to generate this structure chain have been modified since the its last 
-+- usage.                                                                          -+-

@param mustache_structure* structure_chain

-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*****/

@(link_name="mustache_structure_chain_flush")
structureChainFlush :: proc (structChain: ^Structure) ---

/*****
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-

-+- Converts JSON from a file on disk into a mustache parameter chain. -+-
@param mustache_parser* parser
@param mustache_const_slice filename
@param mustache_param** paramRoot - pointer to a pointer to the beginning of the parameter chain.

@return uint8_t - MUSTACHE_RES return code.

-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*****/

@(link_name="mustache_JSON_to_param_chain_from_disk")
JSON_toParamChainFromDisk :: proc(parser: ^Parser, filename: []u8, paramRoot: ^^Param) -> Err ---

/*****
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-

-+- Converts JSON into a mustache parameter chain. -+-

@param mustache_parser* parser
@param mustache_const_slice - JSON source
@param mustache_param** paramRoot - pointer to a pointer 
@param bool deepCopyData - if true, the source data will be copied rather than shallowly referenced where applicable.

@return uint8_t - MUSTACHE_RES return code.

-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*****/

@(link_name="mustache_JSON_to_param_chain")
JSON_toParamChain :: proc(parser: ^Parser, JSON: []u8, paramRoot: ^^Param, deepCopyData: bool) -> Err ---

/*****
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-

-+- Converts JSON into a mustache parameter chain. -+-

@param mustache_parser* parser
@param mustache_const_slice - JSON source
@param mustache_param* paramRoot - the parameter root
@param bool deepCopyData - determines what data will be freed.

@return uint8_t - MUSTACHE_RES return code.

-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*/

@(link_name="mustache_free_param_list")
mustache_free_param_list :: proc(parser: ^Parser, paramRoot: ^Param, deepCopy: bool) -> Err ---

/*
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-

                                 SYSTEM TESTS & DEBUG TOOLS

-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*/


}