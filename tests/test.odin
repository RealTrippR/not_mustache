package test

import nm "../boilerplates/odin/not_mustache"
import "core:mem"
import "core:fmt"

alloc :: proc(parser: nm.Parser, size: int) -> (rawptr) {
    memslice,_ := mem.alloc_bytes_non_zeroed(size)
    return cast(rawptr) &memslice[0]
}


free :: proc(parser: nm.Parser, block: rawptr) {
    mem.free(block)
}


main :: proc() {
    fmt.println("Hello")
    param := nm.ParamString {
        type = .String,
        name = "Name",
        str = "Tripp"
    }
}