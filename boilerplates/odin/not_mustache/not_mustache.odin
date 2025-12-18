/*
-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- 

Robins Free of Charge & Open Source Public License 25

Copyright (C), 2025 - Tripp R. All rights reserved.

Permission for this software, the "software" being source code, binaries, and documentation,
shall hereby be granted, free of charge, to be used for any purpose, including commercial applications,
modification, merging, and redistrubution. The software is provided 'as-is' and comes without any
express or implied warranty. This license is valid under the following restrictions:

1. The origin of the software must not be misrepresentented; the true author(s) of the software
must be attributed as such. This applies every alteration of the "software", the name(s)
of the authors(s) of any alterations must be appended to the list of names of
the author(s) of the preceding version of the software which the alteration is based upon.

2. This license must be included in all redistributions of the software source.

3. All distributions of altered forms of the software must be clearly marked as such.

4. The author(s) of this software and all subsequent alterations hold no responsibility for any
damages that may result from use of the software.

5. The software shall not be used for the purpose of training LLMs ("Large Language Models"),
be included in datasets used for the purpose of training AI, or be used in the advancement of any
form of Artificial Intelligence.

-+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+- -+-
*/
package not_mustache

import "core:odin/parser"
import "core:c"
import "core:math"
import "core:slice"

foreign import libm "libm.lib"

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

Alloc :: proc "c" (parser: ^Parser, size: int) -> (rawptr);

Free :: proc "c" (parser: ^Parser, block: rawptr);

ParseCallback :: proc "c" (parser: ^Parser, udata: rawptr, parsed: []u8);

SeekCallback :: proc "c" (udata: rawptr, whence: i64, seekdir: SeekDir) -> (u64)

ReadCallback :: proc "c" (udata: rawptr, dst: ^u8, dstlen: u64) -> (u64)


/* ====== STRUCTURE TYPES ====== */

Parser :: struct {
    userData: rawptr,
    alloc: Alloc,
    free: Free
}

ParamType :: enum {
    None,
    Boolean,
    Number,
    String,
    List,
    Object,
    Template,
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

ParamTemplate :: struct {
    using param: Param,
    parameters: ^Param,
    structure: ^Structure,
    source: string,
    parentStackBuffer: []u8
}

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
parseFile :: proc (parser: ^Parser, parentStackBuffer: []u8,  filename: string , structChain: ^Structure,  params: ^Param,
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
parseStream :: proc (parser: ^Parser, parentStackBuffer: []u8, stream: ^Stream, structChain: ^Structure,  params: ^Param,
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