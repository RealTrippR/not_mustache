Not Mustache has no strict requirements for the design of templating engines.

The engine included with this project uses a buffered callback system in which a stream object (`mustache_stream`) copies data from a stream into the input buffer fed as an argument into `mustache_parse_stream` or `mustache_parse_file`.

These input chunks are processed by the template engine and passed into the `parseCallback` argument of `mustache_parse_stream` or `mustache_parse_file`. The processed data is then stored in the output buffer.

The `mustache_structure` is an opaque object type used to cache the structure of a template. It improves performance by converting the template into a binary format instead of re-parsing the source template on every call.

To generate this structure, memory is allocated using `parser_alloc` and `parser_free`. The structure is automatically generated on first use and reused for all subsequent parses.

Call `structure_chain_flush` to clear a structure chain if any of the following used to generate it have changed:

* the source template
* parameter addresses
* parameter types

`mustache_structure_chain_free` MUST BE CALLED to free any memory allocated for a `mustache_structure` chain.

Note that `mustache_parse_file` uses `mustache_parse_stream` internally; it only exists to simplify the process of reading template files.

All chunks fed to this engine from a stream must contain only complete top-level scopes; they cannot be split across chunks.

For example:

`/{{#mybool}} {{var}} {{/}}`

is valid, but:

Chunk 1:
`/{{#mybool}} {{var}}`

Chunk 2:
`{{/}}`

is invalid.

This parsing engine does not perform significant validation checks on the correctness of a Not Mustache template. It is not recommended to use untrusted templates with this parsing engine.
