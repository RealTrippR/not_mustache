package test

import "core:odin/parser"
import nm "../boilerplates/odin/not_mustache"
import "core:mem"
import "core:os"
import "base:runtime"

_alloc :: proc "c" (parser: ^nm.Parser, size: int) -> (rawptr) {
    context = runtime.default_context()
    i,_:=mem.alloc(size)
    return  cast(rawptr)i
}


_free :: proc "c" (parser: ^nm.Parser, block: rawptr) {
    context = runtime.default_context()
    mem.free(block)
}

parseCallback :: proc "c" (parser: ^nm.Parser, udata: rawptr, parsed: []u8)
{
    context = runtime.default_context()
    foutHdl := cast(^os.Handle)udata;
    os.write(foutHdl^, parsed)
    return;
}

main :: proc()
{
    i,_:= mem.alloc(100)

    SOURCE_BUFFER := [4096]u8{};
    OUTPUT_BUFFER := [4096]u8{};
    PARENT_STACK_BUFFER := [4096]u8{};

    name := nm.ParamString {
        type = .String,
        name = "name",
        str = "Tripp"
    }

    context_allocator := context.allocator

    parser := nm.Parser {
        userData = &context_allocator,
        alloc = _alloc,
        free = _free,
    }


    foutHdl, foutErr := os.open("example_parsed.txt", os.O_CREATE|os.O_WRONLY|os.O_TRUNC);
    if foutErr!=nil {
        os.exit(-1)}

    structChain: nm.Structure;
    nm.parseFile(&parser, PARENT_STACK_BUFFER[:], "example.txt", &structChain, &name, SOURCE_BUFFER[:], OUTPUT_BUFFER[:], &foutHdl, parseCallback)
}