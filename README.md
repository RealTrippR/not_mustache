# NOT_MUSTACHE

Not-mustache is a near-identical derivative of the mustache templating templating language, and is backwards compatible with most mustache templates. It adds array indexing and length functions, has a shortened syntax, and handles standalone lines differently.

### Specification
Refer to `SPEC.md` for information on the language.

### Examples

Just like with mustache, all blocks are defined with an opening `{{` and a closing `}}`, which an expression in between.
```
/{{name}} - by default, HTML is escaped:
<h3>Hello, {{name}}.</h3>
/{{&name}} - permits raw HTML:
<h3>Hello, {{&name}}.</h3>
```

Not-mustache supports foreach loops and array indexing:
```
{{#users[2]}}
    <p>users[2]: {{users[2].data.name}}</p>
    The 3rd user does exist.
{{else}}
    The 3rd user does not exist.
{{/}}

{{#users}}
<p>{{.data.name}} is online.</p>
{{/}}

```

# Building the C Template Engine

Building the C-based templating engine is fairly straightforward. The makefile assumes that you have gcc installed on your system and
availble in your system path. If you plan to link with MSVC or Odin, define the preprocessor macro `NOT_MUSTACHE_TARGET_MSVC`.

## C Naming Conventions
- Preprocessor Macros: UPPER_SNAKE_CASE
- Function Names: snake_case
- Variable names: pascalCase
- Struct/Union Types: CamelCase
- Enum Types: CamelCase
- Enum Values: UPPER_SNAKE_CASE
